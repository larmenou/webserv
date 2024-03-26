<?php
    session_start();
    if (!isset($_COOKIE["user"])) 
        header('Location: login.php');
?>
<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
</head>
<body>

<?php
        echo "<h1> WELCOME ".$_COOKIE["user"]."!</h1><a href='./login.php?logout'>LOGOUT</a>";
?>

</body>
