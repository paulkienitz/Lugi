create or replace view RanksThisYear as
    select score, whenscored, playername, istest,
           row_number() over (partition by istest, upper(playername)
                              order by score desc, whenscored asc) as personalrank
      from Scores
     where timestampdiff(month, whenscored, now()) < 12
       and score > 0;


delimiter //

create or replace function EntriesPerName(
    p_istest  boolean
) returns int
begin
    declare v_scores, v_namesInUse  int;

   -- how should we display scores if there are few distinct users?
    select count(*), count(distinct playername) into v_scores, v_namesInUse
      from Scores
     where istest = p_istest;
    if v_scores <= 10 then
        return 10;
    elseif v_namesInUse >= 10 then
        return 1;
    elseif v_namesInUse >= 6 then
        return 2;
    else
        return 3;
    end if;
    -- in testing, this fudgery seems to work intuitively enough
end;
//


create or replace function ScoreNeedsName(
    p_score   int,
    p_istest  boolean
) returns boolean
begin
    declare v_min, v_maxpername  int;

    set v_maxpername = EntriesPerName(p_istest);
    select min(score) into v_min from RanksThisYear
     where personalrank <= v_maxpername
       and istest = p_istest
     order by score desc, whenscored asc
     limit 10;

    return p_score > 0 and (v_min is null or v_min < p_score);
end;
//


create or replace procedure CreateToken()
begin
    declare v_toke  varchar(40);

    delete from Tokens
     where timestampdiff(day, whenadded, now()) > 7;

    set v_toke = uuid();
    insert into Tokens (token) value (v_toke);
    select v_toke;
end;
//


create or replace procedure LugiScorage(
    in  p_token        varchar(64),
    in  p_playername   varchar(64),
    in  p_when         varchar(20),    -- ISO date format except no timezone
    in  p_score        int,
    in  p_ip           varchar(40),
    in  p_useragent    varchar(500),
    in  p_istest       boolean,
    out p_yearplace    int,
    out p_alltimeplace int,
    out p_bottomplace  boolean
)
begin
    declare v_maxpername, v_ct  int;
    declare v_when  datetime;
    declare v_token  varchar(40);
    declare exit handler for sqlexception begin
        rollback;
        resignal;
    end;

    start transaction;

    -- preliminary: validate origin of request
    select count(*) into v_ct from Tokens
     where token = p_token;
    if v_ct = 0 then
        signal sqlstate '42000' set message_text = 'token not found';
    end if;
    delete from Tokens where token = p_token;  -- no reuse

    -- PART 1: check if the score has a new rank among the worst and best
    set v_maxpername = EntriesPerName(p_istest);

    select count(*) into v_ct from Scores
     where score <= p_score
       and istest = p_istest;  -- and score < 0?
    set p_bottomplace = if(v_ct > 0, -1, v_ct);

    select count(*) into v_ct from Scores
     where score >= p_score
       and istest = p_istest
       and score > 0;
    set p_alltimeplace = if(v_ct > 2 or p_score <= 0, -1, v_ct);

    select count(*) into v_ct from RanksThisYear   -- use the view
     where istest = p_istest
       and score >= p_score
       and personalrank <= v_maxpername;
    set p_yearplace = if(v_ct > 9 or p_score <= 0, -1, v_ct);

    if p_yearplace >= 0 then
        select count(*) into v_ct from RanksThisYear
         where score >= p_score
           and istest = p_istest
           and upper(playername) = upper(p_playername);
        set p_yearplace = if(v_ct >= v_maxpername, -2, p_yearplace);
    end if;

    -- PART 2: log the new score
    set v_when = str_to_date(p_when, '%Y-%m-%dT%H:%i:%s');
    insert into Scores (playername, whenscored, score, ip, useragent, istest)
           values (p_playername, v_when, p_score, p_ip, p_useragent, p_istest);
    -- the columns score_id and servertime are filled in automatically

    -- PART 3: return the updated list of 13 bests and one worst
    with TopTen as (
        select 'Y' as sect, score, whenscored, playername, istest,
               row_number() over (order by score desc, whenscored asc) as sorty
          from RanksThisYear
         where personalrank <= v_maxpername
           and istest = p_istest
         order by sorty
         limit 10
    ), TopThree as (
        select 'A' as sect, score, whenscored, playername, istest,
              row_number() over (order by score desc, whenscored asc) + 9000 as sorty
          from Scores
         where istest = p_istest
           and score > 0
         order by sorty
         limit 3
    ), Bottom as (
        select 'B' as sect, score, whenscored, playername, istest, 9999 as sorty
          from Scores
         where istest = p_istest
         order by score asc, whenscored asc
         limit 1
    )
    select sect, score, date_format(whenscored, '%Y-%m-%dT%T') as whenscored, playername, istest, sorty from TopTen
     union
    select sect, score, date_format(whenscored, '%Y-%m-%dT%T') as whenscored, playername, istest, sorty from TopThree
     union
    select sect, score, date_format(whenscored, '%Y-%m-%dT%T') as whenscored, playername, istest, sorty from Bottom
     order by sorty;

    commit;
end;
-- sample of how to invoke:
-- call CreateToken();
-- call LugiScorage('xxx-xxx.... (returned token)', 'Roger', '2023-04-20T12:34:05', 11, null, null, 1, @year, @all, @bot);
-- select @year, @all, @bot;
//
