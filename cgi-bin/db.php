<?php

$db = array("JohnCena" => array(sha1('secretpassword'), 'https://images.cinefil.com/stars/1056719.webp'), 
            "Obama" => array(sha1('password123'), 'https://upload.wikimedia.org/wikipedia/commons/thumb/8/8d/President_Barack_Obama.jpg/800px-President_Barack_Obama.jpg')
        );

function    checkPassword($username, $password)
{
    print_r($username);
    if (!isset($db[$username]))
        return false;
    if (sha1($password) === $db[$username][0])
        return true;
    return false;
}
?>