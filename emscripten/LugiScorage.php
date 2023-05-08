<?php

// This is a service API for saving and retrieving scores for the game of Lugi.
// It is used by a web-accessible game of Lugi hosted on the same server.
// It's basically a wrapper for stored procedures such as LugiScorage.

require 'LugiDB.php';


define('zonelessIsoFormat', 'Y-m-d\TH:i:s');

$never   = new DateTimeImmutable('1900-01-01');
$mindate = new DateTimeImmutable('2022-01-01');  // make this yesterday?
$maxdate = new DateTimeImmutable('2122-01-01');  // make this tomorrow?
$testmode = false;


class Score {
    function __construct(string $who = '', ?DateTimeInterface $when = null, int $howMuch = 0)
    {
        global $never;
        $this->who = $who;
        $this->when = $when ?? $never;
        $this->howMuch = $howMuch;
    }
    public string            $who;          // player name
    public DateTimeInterface $when;         // local time game completed
    public int               $howMuch;      // score
}

class ScoreExport {
    function __construct(Score $s)
    {
        $this->who = $s->who;
        $this->when = $s->when->format(zonelessIsoFormat);
        $this->howMuch = $s->howMuch;
    }
    public string $who;
    public string $when;
    public int    $howMuch;
}


function err(string $msg)
{
    http_response_code(400);
    echo json_encode($msg);
}


function fail(string $msg, ?Throwable $ex)
{
    if ($ex)
        error_log(formatThrowable($ex));
    http_response_code(500);
    echo json_encode($msg);
}


function scoresFromRows(array &$rows)
{
    $scores = array_fill(0, 14, new Score());
    // the source row collection may have fewer rows than the destination,
    // so we leave pre-initialized empties in place as needed
    $scoreix = 0;
    foreach ($rows as $row) {
        if ($scoreix >= 14)
            break;
        else if ($scoreix < 13 && $row['sect'] == 'B')
            $scoreix = 13;
        else if ($scoreix < 10 && $row['sect'] == 'A')
            $scoreix = 10;
        else if ($scoreix >= 10 && $row['sect'] == 'Y')
            break;  // also allow for excess rows -- skip them
        else if ($scoreix >= 13 && $row['sect'] == 'A')
            break;
        $when = DateTimeImmutable::createFromFormat('!' . zonelessIsoFormat, $row['whenscored']);
        if ($when)
            $scores[$scoreix++] = new Score($row['playername'], $when, $row['score']);
    }
    return $scores;
}


function addScoreAndReturnList(string $token, string $playerName, string $when, int $score, ?string $ipAddress, ?string $userAgent)
{
    global $testmode;
    try {
        $context = '* could not connect to database';
        $db = connect();

        $context = '* could not call score procedure';
        $stmt = $db->prepare('call LugiScorage(?, ?, ?, ?, ?, ?, ?, @yearpos, @alltimepos, @bottompos)');
        $stmt->bind_param('sssissi', $token, $playerName, $when, $score, $ipAddress, $userAgent, $testmode);
        $context = '* exception in score procedure';
        $stmt->execute();

        $context = '* could not retrieve score records';
        $zult = $stmt->get_result();
        $rows = $zult->fetch_all(MYSQLI_ASSOC);
        $context = '* could not read score records';
        $scores = scoresFromRows($rows);
        $stmt->close();

        // consider the option of tolerating failure here and just setting $yearpos = $alltimepos = $bottompos = -1?
        $context = '* could not retrieve out-parameter values';
        $zult = $db->query('select @yearpos, @alltimepos, @bottompos');
        $row = $zult->fetch_assoc();
        $context = '* could not read out-parameter values as ints';
        $yearpos    = (int) $row['@yearpos'];
        $alltimepos = (int) $row['@alltimepos'];
        $bottompos  = (int) $row['@bottompos'];
        // for some reason $stmt->close() is an error here, though necessary above to avoid "Commands out of sync"

        $context = '* could not produce score output json';
        $response = new stdClass();
        $response->yearpos    = $yearpos;
        $response->alltimepos = $alltimepos;
        $response->bottompos  = $bottompos;
        $response->scores     = array_map(function ($score) { return new ScoreExport($score); }, $scores);

        echo json_encode($response);
    } catch (Throwable $e) {
        fail($context, $e);
    } finally {
        disconnect($db);
    }
}


function checkNameNeeded(int $score)
{
    global $testmode;
    try {
        $tm = (int) $testmode;
        $context = '* database connection for name check failed';
        $db = connect();
        $context = '* could not call name check function';
        $zult = $db->query("select ScoreNeedsName($score, $tm)");
        $context = '* could not extract name check function return value';
        echo json_encode((bool) ($zult->fetch_row()[0]));
    } catch (Throwable $e) {
        fail($context, $e);
    } finally {
        disconnect($db);
    }
}


function echoNewToken()
{
    try {
        $context = '* database connection for token failed';
        $db = connect();
        $context = '* could not call token procedure';
        $zult = $db->query("call CreateToken()");
        $context = '* could not extract token value';
        echo json_encode($zult->fetch_row()[0]);
    } catch (Throwable $e) {
        fail($context, $e);
    } finally {
        disconnect($db);
    }
}


function testConnect()
{
    try {
        $db = connect();
        echo json_encode(true);
    } catch (Throwable $e) {
        fail('* database connection test failed', $e);
    } finally {
        disconnect($db);
    }
}


function dumpServerVars()
{
    $keyfilter = function ($key) {
        return (strpos($key, 'REMOTE_') === 0 || strpos($key, 'HTTP_') === 0)
               && $key != 'HTTP_COOKIE';
    };
    echo json_encode(array_filter($_SERVER, $keyfilter, ARRAY_FILTER_USE_KEY), JSON_PRETTY_PRINT);
}


// ================ MAIN:

setlocale(LC_ALL, 'en_US.UTF8');
header('Content-Type: application/json; charset=utf-8');
$op = $_GET['op'];

if ($op == 'addnlist') {
    $to = $_GET['to'];
    $pn = $_GET['pn'];
    $ts = $_GET['ts'];
    $sc = $_GET['sc'];
    $ip = $_SERVER['HTTP_CLIENT_IP'] ?? $_SERVER['HTTP_X_FORWARDED_FOR'] ?? $_SERVER['REMOTE_ADDR'];
    $ua = $_SERVER['HTTP_USER_AGENT'];

    if (!$to)
        err('** to required');
    else if (!$pn)
        err('** pn required');
    else if (!$sc)
        err('** sc required');
    else if (!is_numeric($sc))
        err("** sc does not parse ($sc)");
    else if (!$ts)
        err('** ts required');
    else {
        $sc = (int) $sc;
        $tsi = DateTimeImmutable::createFromFormat('!' . zonelessIsoFormat, $ts);
        if (!$tsi)
            err("** ts does not parse - '$ts'");
        else if ($tsi < $mindate || $tsi > $maxdate)
            err('** ts out of range - ' . $tsi->format('Y'));
        else
            addScoreAndReturnList($to, $pn, $ts, $sc, $ip, $ua);
    }
} else if ($op == 'needname') {
    $sc = $_GET['sc'];
    if (!$sc)
        err('** sc required');
    else if (!is_numeric($sc))
        err("** sc does not parse ($sc)");
    else
        checkNameNeeded((int) $sc);
} else if ($op == 'gettoken' && $testmode)
    echoNewToken();
else if ($op == 'dumpy' && $testmode)
    dumpServerVars();
else if ($op == 'testconn')
    testConnect();
else
    err('** operation not recognized');

?>