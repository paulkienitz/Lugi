<?php
require 'LugiDB.php';

$token = '';

try {
    $db = connect();
    $zult = $db->query("call CreateToken()");
    $token = json_encode($zult->fetch_row()[0]);
} catch (Throwable $ex) {
    error_log(formatThrowable($ex));
} finally {
    disconnect($db);
}
?><!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1'/>
    <meta charset="UTF-8">
    <title>the game of Lugi</title>

    <style>
        body {
            background-color: #eef7ee;
            background: linear-gradient(to right, #d7eed7, #eef3ee, #eef3ee, #d7eed7);
            padding: 1px 0;      /* no margin collapse */
            margin: 0 8px;
        }
        a:link, a:visited {
            color: #0000cc;
        }
        #banner {
            text-align: center;
            font-style: italic;
            font-size: 21px;
            font-family: "Georgia", "Times New Roman", Times, serif;
        }

        #overscan {
            font-family: "Lucida Console", "Consolas", "Source Code Pro", "Source Code", monospace;
            font-size: 16px;
            text-size-adjust: none;     /* stop Chrome/A from being a butthead */
            padding: 1.5ch;
            margin: 1em auto;
            max-width: 84ch;
            background-color: #000000;
        }     /* parent controls child's width, child controls parent's height */
        #console {
            width: 100%;
            height: 24em;               /* will be corrected by handleResize */
            overflow-y: scroll;
            -webkit-text-size-adjust: none;
            box-sizing: content-box;
            position: relative;         /* does thus have a reason to be here anymore? */
            caret-color: transparent;   /* workaround for a Firefox bug when input is empty */
        }
        #console::-webkit-scrollbar {   /* this loses buttons... but -webkit-scrollbar-track does not work at all?? */
            background-color: #333333;
        }
        #console::-webkit-scrollbar-thumb {
            background-color: #777777;
        }
        #output, #prompt {
            color: #dddddd;
            white-space: pre-wrap;
        }
        #input {
            color: #00ff00;
            background-color: #000000;
            caret-color: transparent;
            border: none;
            font-family: inherit;
            font-size: 1em;
        }
        #input:focus-visible {
            outline: none;
            caret-color: transparent;
        }
        #input::selection {
            background-color: #0077ee;
            color: #ffffff;
        }
        #carrot {
            font-family: "Lucida Console", "Consolas", "Source Code Pro", "Source Code", monospace;
            font-size: 16px;
            position: absolute;
            z-index: 1;
            background-color: #44ee44;
            visibility: hidden;
            animation: 0.6s steps(2, jump-none) infinite blinky;
        }
        @keyframes blinky {
            from { opacity: 0; }
            to   { opacity: 75%; }
        }
        #output .event   { color: #00ffff; }
        #output .command { color: #00ff00; }
        #output .alarm   { color: #ffd700; }
        #output .debug   { color: #ff00ff; }
        #morePrompt      { color: #000000; background-color: #dddddd; }

        #measurator {
            visibility: hidden;
            position: absolute;
            z-index: -1;
            white-space: nowrap;
        }

        #lynx {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            column-gap: 1en;
            justify-content: space-between;
            max-width: 800px;
            margin: 1em auto;
            font-family: "Georgia", "Times New Roman", Times, serif;
            font-style: italic;
            font-size: 16px;
        }
        #linkLeft {
            justify-self: start;
            text-align: left;
        }
        #linkCenter {
            justify-self: center;
            text-align: center;
        }
        #linkRight {
            justify-self: end;
            text-align: right;
        }

        /* try to fit at least 52 chars across so the score is legible... for the
           smaller widths we track it pretty closely, for larger ones we don't have to */
        @media (max-width: 350px) {                          /* early phones were often in this range */
            #overscan, #carrot { font-size: 9px; }           /* (320 is the smallest size normally used?) */
            #banner { font-size: 12px; }
            #lynx { font-size: 10px; }
        }
        @media (min-width: 350px) and (max-width: 390px) {   /* many current phones fall in this range */
            #overscan, #carrot { font-size: 10px; }          /* (360 is common as it divides 1080 and 1440 evenly) */
            #banner { font-size: 13px; }
            #lynx { font-size: 11px; }
        }
        @media (min-width: 390px) and (max-width: 430px) {   /* my phone falls in this range */
            #overscan, #carrot { font-size: 11px; }
            #banner { font-size: 14px; }
            #lynx { font-size: 12px; }
        }
        @media (min-width: 430px) and (max-width: 520px) {   /* these middle ranges aren't used very much, */
            #overscan, #carrot { font-size: 12px; }
            #banner { font-size: 15px; }
            #lynx { font-size: 13px; }
        }
        @media (min-width: 520px) and (max-width: 640px) {   /* ...but desktop windows might end up sized here */
            #overscan, #carrot { font-size: 14px; }
            #banner { font-size: 18px; }
            #lynx { font-size: 14px; }
        }
    </style>

    <script>
        // future: try to do something about viewport height in Chrome for Android with keyboard up?
        var output, prompt, input, carrot;
        var outputClass = '';
        var inputResolver = null, anyKeyResolver = null;
        var pxCharWidth, pxLineHeight;
        var beepSound = new Audio('hiding.wav');    // or 'beep-08b.wav'
        var playerName = "";
        var authenticationToken = [  <?php echo $token; ?> ];

        function spanout(content, classname)
        {
            let s = document.createElement('span');
            s.innerText = content;
            if (classname)
                s.className = classname;
            output.appendChild(s);
        }

        function say(content)
        {
            if (!output.children.length && content == '\n')
                return;
            spanout(content, outputClass);
            input.scrollIntoView();
        }

        async function ask(promptText)
        {
            prompt.innerText = promptText;
            activateInput(true);
            let response = await new Promise((resolve, reject) => inputResolver = resolve);
            prompt.innerText = "";
            spanout(promptText);
            spanout(response.trimEnd() + "\n", 'command');
            return response;
        }

        function promptForMore()
        {
            prompt.innerHTML = "<span id=morePrompt> -- MORE -- </span> ";
        }

        async function waitForKeypressAndClearPrompt()
        {
            input.innerText = "";
            activateInput(true);
            await new Promise((resolve, reject) => anyKeyResolver = resolve);
            prompt.innerText = "";
        }

        function activateInput(active)
        {
            // originally this enabled and disabled a textbox
            if (active) {
                input.scrollIntoView();
                selectHandler();
                setTimeout(() => input.focus(), 50);
            } else {
                input.innerText = "";
                carrot.style.visibility = 'hidden';
            }
        }

        function keyHandler(e)
        {
            // A fact that is the opposite of fun: on Android, if the soft keyboard has
            // suggestions enabled, it's treated as an IME in which whitespace keys
            // complete composition, and therefore Enter never appears in a KeyEvent!
            // So we have to check the compositionend event (or input, but Chrome sends that twice).
            let isEnter = e.key == 'Enter' || e.keyCode == 13 || (e.data && e.data.endsWith('\n'));
            if (inputResolver && isEnter) {
                inputResolver(input.innerText);
                inputResolver = null;        // idempotency
                activateInput(false);
            } else if (anyKeyResolver) {
                anyKeyResolver();
                anyKeyResolver = null;
                e.preventDefault();
                activateInput(false);
            }
            if (isEnter)
                e.preventDefault();
            if (e.keyCode == 8 || e.key == 'Backspace')
                setTimeout(selectHandler, 10);          // Chrome/W fails to send selectionchanged event
        }

        function clickHandler(e)
        {
            input.focus();
        }

        function resizeHandler()
        {
            measurate();       // media rules may have changed font size
            let unusedVerticalSpace = window.innerHeight - document.body.getBoundingClientRect().height - 1.0;
            let moreLines = Math.floor(unusedVerticalSpace / pxLineHeight);    // may be negative
            let newLineCount = Math.min(50, Math.max(9, consoleHeight() + 1 + moreLines));
            if (moreLines || !output.parentElement.style.height)   // height is blank the first time
                output.parentElement.style.height = Math.floor(newLineCount * pxLineHeight) + "px";
                // Math.ceil can leave descender specks showing sometimes
            input.scrollIntoView();
            selectHandler();
        }

        function selectHandler()
        {
            let s = window.getSelection();
            if (document.activeElement == input && s && s.isCollapsed && s.rangeCount == 1
                            && (s.focusNode == input || s.focusNode.parentElement == input)) {
                // the caret in input has moved, either during typein (in which case we are called as a handler)
                // or by other layout changes (in which case we have called this manually, e.g. in activateInput)
                let b = s.getRangeAt(0).getBoundingClientRect();    // caret location with zero width and full height
                if (!b.y && !b.height)                              // ...except if input is empty, then that don't work
                    b = input.getBoundingClientRect();
                carrot.style.visibility = 'visible';
                carrot.style.width = pxCharWidth + 'px';
                carrot.style.height = pxLineHeight + 'px';          // any less height messes up either desktop or mobile,
                carrot.style.left = b.left + 'px';                  // as they don't agree whether the excess is top or bottom
                carrot.style.top = b.top + 'px';
            } else
                carrot.style.visibility = 'hidden';
        }

        function setClassOfSaying(cssClass)
        {
            outputClass = cssClass;
        }

        function getCookie(name)
        {
            let re = new RegExp('(^|\\W)' + name + '=((\\w*)|\"([^\"]*)\")($|;)');
            let ma = re.exec(document.cookie);
            return !ma || !ma.length ? null : decodeURIComponent(ma[4] || ma[3] || "");
        }

        function setPersistentCookie(name, contents)
        {
            document.cookie = name + '="' + encodeURIComponent(contents) +
                              '";max-age=' + (5 * 365 * 86400) + ";samesite=Strict";
        }

        function z2(num)
        {
            // the date parser in PHP tolerates lack of leading zeros in some fields,
            // but not in minutes or seconds for some dumb reason, so let's add them
            return parseInt(num).toString().padStart(2, "0");
        }

        async function callService(url, timeout)
        {
            let bortus = new AbortController();
            setTimeout(() => bortus.abort(), timeout * 1000);
            try {
                let response = await fetch(url, { signal: bortus.signal });
                if (response.ok)
                    return await response.text();
                else
                    console.error("Service responded " + response.status + " " + await response.text());
            } catch (e) { }
            return null;
        }

        async function saveToServerAndGetScores(who, y, m, d, h, n, s, howmuch)
        {
            // tested names like "aaa ëêē π√×¶∆ Луги 盧記 🕹️🚻🥋" in-game to verify utf8 end to end...
            // the one problem observed is that they can word-wrap early
            try {
                let time = parseInt(y) + "-" + z2(m) + "-" + z2(d) + "T" + z2(h) + ":" + z2(n) + ":" + z2(s);
                let args = "&sc=" + howmuch + "&ts=" + time + "&pn=" + encodeURIComponent(who) + "&to=" + authenticationToken[0];
                let scoreState = JSON.parse(await callService("./LugiScorage.php?op=addnlist" + args, 10));
                scoreState.formattedScores = scoreState.scores.map(s => s.howMuch + " " + s.when + " |" + s.who + "\n").join("");
                return scoreState;
            } catch (e) {
                return null;
            }
        }

        async function testDatabaseAccess()
        {
            return authenticationToken[0] && await callService("./LugiScorage.php?op=testconn", 2) != null;
        }

        async function doesScoreNeedName(score)
        {
            return JSON.parse(await callService("./LugiScorage.php?op=needname&sc=" + parseInt(score), 4));
        }

        function consoleWidth()
        {
            let r = output.getBoundingClientRect();    // does not include scrollbar
            return Math.floor(r.width / pxCharWidth);
        }

        function consoleHeight()
        {
            let r = output.parentElement.getBoundingClientRect();
            return Math.floor(r.height / pxLineHeight - 0.75);   // don't count the line for prompt and input, unless the line scrolled off the top is mostly still visible
        }

        function measurate()
        {
            let b1 = document.getElementById('measur1').getBoundingClientRect();
            let b2 = document.getElementById('measur2').getBoundingClientRect();
            pxCharWidth = b1.width / 20.0;
            pxLineHeight = (b2.bottom - b1.bottom) / 5.0;
        }

        function beep()
        {
            if (beepSound)
                beepSound.play();
        }

        function quit()
        {
            input.scrollIntoView();
            input.style.display = 'none';
            carrot.style.display = 'none';
        }

        async function clearOutput()
        {
            // verify that browser understands async syntax by clearing message
            output.innerText = "";
        }
    </script>
</head>


<body>
    <div id=carrot></div>
    <p id=banner>
        Let&rsquo;s play <strong>Lugi</strong> in a monospace scrolling
        text window, as the Gods of Arcturus IV intended!
    </p>

    <div id=overscan>
        <div id=console>
            <div id=measurator>
                <span id=measur1>00000000000000000000</span>
                <br/> <br/> <br/> <br/> <br/>
                <span id=measur2>0</span>
            </div>
            <div id=output>Ironically, Lugi requires a modern browser.</div>
            <span id=prompt></span><span id=input contenteditable=true autofocus></span>
        </div>
    </div>

    <div id=lynx>
        <div id=linkLeft><a href='./'>about the game</a></div>
        <div id=linkCenter><a href='http://paulkienitz.net'>paulkienitz&#8203;.net home</a></div>
        <div id=linkRight><a href='mailto:paul@paulkienitz.net?subject=Lugi'>email Paul</a></div>
    </div>

    <script defer>
        output = document.getElementById('output');
        prompt = document.getElementById('prompt');
        input  = document.getElementById('input');
        carrot = document.getElementById('carrot');
        input.innerText = "";
        if (clearOutput) clearOutput();
        if (beepSound) beepSound.volume = 0.5;

        input.addEventListener('keydown', keyHandler);
        input.addEventListener('compositionend', keyHandler);
        document.addEventListener('selectionchange', selectHandler);
        input.addEventListener('focus', selectHandler);
        window.addEventListener('click', clickHandler);  // sets focus
        window.addEventListener('resize', resizeHandler);
        setTimeout(resizeHandler, 0);
    </script>

    <script defer type='text/javascript' src='Lugi.js'></script>
</body>
</html>

 