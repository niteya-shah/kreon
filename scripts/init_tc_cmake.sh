#!/bin/bash

module load gcc/8.2.0
module load GCCcore
module load Boost

export UMAP_LIB=/projects/synergy_lab/umap/apps/umap/build/lib/libumap.so
export UMAP_INCL=/projects/synergy_lab/umap/apps/umap/build/include/umap
/beegfs/projects/synergy_lab/umap/apps/cmake-3.21.2-linux-x86_64/bin/cmake .. \
        -DUSE_UMAP=1

make -j 20
