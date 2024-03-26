<?php
    if (!isset($_COOKIE["user"])) 
        header('Location: login.php');
?>
<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
</head>
<body>

<div style="text-align: center;">

<?php
    if (isset($_COOKIE["user"])) 
    {
        echo "<img src='https://upload.wikimedia.org/wikipedia/commons/thumb/8/8d/President_Barack_Obama.jpg/800px-President_Barack_Obama.jpg' height=500px>";
        echo "<h1> WELCOME President ".$_COOKIE["user"]."!</h1><a href='./login.php?logout'>LOGOUT</a>";
    }

?>

</div>

</body>
