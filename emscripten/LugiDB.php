<?php
define('usage', '--your DB name here--');
define('passage', '--your DB password here--');

function connect(): mysqli
{
    mysqli_report(MYSQLI_REPORT_ERROR | MYSQLI_REPORT_STRICT);   // throw exceptions
    $db = new mysqli('localhost', usage, passage, usage);
    $db->set_charset('utf8mb4');
    return $db;
}

function disconnect(?mysqli $db)
{
    if ($db) $db->close();
}


function formatThrowable(?Throwable $ex): ?string
{
    if (!$ex)
        return null;
    $type = get_class($ex) . ($ex->getCode() ? ' (' . $ex->getCode() . ')' : '');
    $line = str_replace($_SERVER['DOCUMENT_ROOT'], '', $ex->getFile()) . ' line ' . $ex->getLine();
    $inner = $ex->getPrevious() ? "\n---- Caused by:\n" . formatThrowable($ex->getPrevious()) : '';
    return $type . ' at ' . $line . ': ' . $ex->getMessage() . $inner;
}
?>
