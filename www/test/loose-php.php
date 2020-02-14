<?php

header('Content-Type: text/html; charset=utf-8');

echo "PHP outside cgi-bin (loose.php=1)</br>";
echo "IP: " . $_SERVER["REMOTE_ADDR"];

?>