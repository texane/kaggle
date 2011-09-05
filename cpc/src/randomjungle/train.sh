#!/usr/bin/env sh
./a.out \
--file=$HOME/repo/kaggle/cpc/data/all_valid_nonzero.csv \
--delimiter=, \
--ntree=75 \
--impmeasure=1 \
--varnamesrow=0 \
--colselection=train.cols \
--votes \
--verbose \
--outprefix=o \
--depvarname=Claim_Amount \
--summary \
--write=2

# --nrow=5000