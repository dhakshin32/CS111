#!/bin/bash

# NAME: DHAKSHIN SURIAKANNU
# EMAIL: bruindhakshin@g.ucla.edu
# ID: 605280083

`touch in.txt`
`echo test_text > in.txt`
`./lab0 --input=in.txt --output=out.txt`

if [ $? -eq 0 ]
then
    echo "Program executed successfully with exit code of 0"
else
    echo "Error: Program did not exit correctly"
fi

`cmp in.txt out.txt`

if [ $? -eq 0 ]
then
    echo "Program works: input matches output"
else
    echo "Error: input does NOT match output"
fi

`./lab0 --input=in.txt --segfault --catch`

if [ $? -eq 4 ]
then
    echo "Program created and caught segmentation fault"
else
    echo "Error: --segfault and --catch did not produce correct exit code"
fi

`./lab0 --input=invalid.txt`

if [ $? -eq 2 ]
then
    echo "Program successfully caught invalid file"
else
    echo "Error: Program did not catch invalid file error"
fi

`touch out.txt`
`chmod u=r out.txt`
`./lab0 --input=in.txt --output=out.txt`

if [ $? -eq 3 ]
then
    echo "Program successfully caught insufficient write permissions to out.txt"
else
    echo "Error: Program did not catch insufficient permissions to write to out.txt"
fi

`rm -f *.txt`
