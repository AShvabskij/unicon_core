#!/bin/bash

PORT=6435
FLG=true

while ${FLG};
do
nc -ulp ${PORT};
sleep 1;
FLG=true;
done

