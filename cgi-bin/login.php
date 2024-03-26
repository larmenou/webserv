<?php 
session_start();
$_SESSION = array();
$users = array("Obama" => sha1("password123"), "user" => sha1("motdepassesecret"));
sleep(1);
if (isset($_GET["logout"]))
{
    header('Location: login.php');
    setcookie ("user", "", time() - 3600);
}

if (isset($_POST['username']) && isset($_POST['password']))
{
    $username = $_POST['username'];
    $password = $_POST['password'];
    if (!isset($users[$username]))
        header('Location: ./login.php?error=1');
    else if ($users[$username] === sha1($password))
    {
        header('Location: ./greet.php');
        setcookie("user", $username);
    }
    else
        header('Location: ./login.php?error=1');
}

if (isset($_COOKIE["user"]))
    header("Location: ./greet.php");
?>

<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
</head>
<body>

<form action="login.php" method="post">
  <div>
    <label for="username"> Username : </label>
    <input type="text" name="username" id="username" required />
  </div>
  <div>
    <label for="password">Password : </label>
    <input type="password" name="password" id="password" required />
  </div>
  <div >
    <input type="submit" value="Login" />
  </div>
</form>

</body>
