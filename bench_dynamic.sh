#!/bin/bash
#SBATCH --job-name=graphCrawler_bench
#SBATCH --error=graphCrawler_%j.err
#SBATCH --time=05:00:00
#SBATCH --partition=Centaurus
#SBATCH --mem=10G

srun $HOME/Dynamic-Graph-Crawler/dynamic "Tom Hanks" 2
srun $HOME/Dynamic-Graph-Crawler/dynamic "Tom Hanks" 3
srun $HOME/Dynamic-Graph-Crawler/dynamic "Tom Hanks" 4
srun $HOME/Dynamic-Graph-Crawler/sequential "Tom Hanks" 2
srun $HOME/Dynamic-Graph-Crawler/sequential "Tom Hanks" 3
srun $HOME/Dynamic-Graph-Crawler/sequential "Tom Hanks" 4


