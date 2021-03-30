#!/bin/bash

{ echo "START"; sleep 2; echo "STOP"; sleep 2; echo "OFF"; } | ./lab4b --log=out.txt --period=2 --scale=C

if [ $? -ne 0 ]
then
	echo "Error: program exit failure"
else
	echo "Smoke test passed"
fi