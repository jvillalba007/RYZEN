#!/bin/bash

# positional arguments
folder="$1"
blocks="$2"

create_blocks(){
	cd bloques
	for i in `seq 1 $blocks`
	do
	    touch "$i".bin	
	    echo "$i".bin created
	done
	cd ..
}

create_metadata(){
	cd Metadata
	echo "BLOCK_SIZE=64
BLOCKS=$blocks
MAGIC_NUMBER=LISSANDRA" > Metadata.bin
	echo Metadata.bin created
	cd ..
}

create_bitmap(){
	cd Metadata
	printf '0%.0s' $(seq 1 $blocks) > Bitmap.bin
	cd ..
}

# Main
echo $blocks blocks
echo folder is $folder

mkdir $folder
cd $folder

mkdir tables
mkdir bloques
mkdir Metadata

create_blocks
create_metadata
# create_bitmap