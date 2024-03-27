<?php 
include "db.php";
session_start();
$_SESSION = array();

if (isset($_GET["logout"]))
{
    header('Location: login.php');
    setcookie ("user", "", time() - 3600);
}
if (isset($_POST['username']) && isset($_POST['password']))
{
    if (sha1($_POST['password']) === $db[$_POST['username']][0])
    {
        header('Location: ./index.php');
        setcookie("user", $_POST['username']);
    }
    else
        header('Location: ./login.php?error=1');
}

if (isset($_COOKIE["user"]))
    header("Location: ./index.php");
?>

<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
</head>
<body>

<div style="text-align: center;">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/4/4d/White_House%2C_Blue_Sky.jpg/271px-White_House%2C_Blue_Sky.jpg">
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

<?php
if (isset($_GET["error"]) && $_GET["error"] == 1)
  echo  "<p> Bad password/username </p>";
?>

</div>

</body>
