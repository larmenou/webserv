<?php
    if (!isset($_COOKIE["user"])) 
        header('Location: login.php');
    session_start();
?>
<!DOCTYPE html>
<html>
<head>
    <title>Welcome</title>
</head>
<body>

<div style="text-align: center;">

<?php

    include("db.php");

    if (isset($_COOKIE["user"])) 
    {
        echo "<img src='".$db[$_COOKIE["user"]][1]."' height=500px>";
        echo "<h1> WELCOME President ".$_COOKIE["user"]."!</h1><a href='./login.php?logout'>LOGOUT</a>";
    }
?>
    <a href='./test.php'>PHPINFO</a>

</div>

</body>
