-- create|alter database Lugi default character set utf8mb4 collate utf8mb4_unicode_ci;


create table Tokens (
    whenadded   datetime     not null default current_timestamp(),
    token       varchar(40)  not null,
    key tokenx  (whenadded)
);


create table Scores (
    score_id    int          not null auto_increment,
 -- token       varchar(40)  default null,
    playername  varchar(64)  not null,
    whenscored  datetime     not null,   -- player's local time
    score       int          not null,
    ip          varchar(40)  default null,
    useragent   varchar(500) default null,
    istest      tinyint(1)   not null default 0,
    servertime  datetime     not null default current_timestamp(),
    primary key (score_id),
    key chronx  (whenscored),
    key namex   (playername)
);
