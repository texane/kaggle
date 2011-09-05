#!/usr/bin/env sh
./a.out \
--predict=o.jungle.xml \
--file=$HOME/repo/kaggle/cpc/data/test_set_reals.csv \
--delimiter=, \
--varnamesrow=0 \
--colselection=predict.cols \
--depvarname=Claim_Amount \
--outprefix=o \
--summary \
--verbose

