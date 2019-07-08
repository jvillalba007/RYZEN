#!/bin/bash
#chmod +x RYZEN.sh

REPONAME=RYZEN

function_clonar() {
    url=https://github.com/sisoputnfrba/tp-2019-1c-RYZEN.git;
    reponame=$1;
    git clone --recursive $url $reponame;
    cd $REPONAME;
    sudo make cinstall
}

function_compilar(){
    make compilar
}

function_clonar "$REPONAME"
function_compilar