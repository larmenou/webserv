<!DOCTYPE html>
<html>
<head>
    <title>Test GET/POST</title>
</head>
<body>
<?php
    print_r($_GET);
    print_r($_POST);
    echo "RAW POST : \n";
    print_r(file_get_contents('php://input'));
    echo "\n\n";
    phpinfo();
?>

</body>