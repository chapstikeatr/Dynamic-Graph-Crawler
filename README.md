Homework for my para Comp class
This group of files shows the time diffrences for 2 types of graphCrawler.

first we run our make file for each of the files (Seq and dynamic) and you need to make sure that you have a local version of rapidjson in the file or make will note work.

if rapidjson is not in the directory use
git clone <https://github.com/Tencent/rapidjson.git>
in the directory and the make file will handle the rest.

Ex.) ./seqLC "Tom Hanks" 2
you provide a starting node and a depth. This is the seqential form and follows the format of ./seqLC <start> <Depth>

you will use the same syntax for dynamic

EX.) ./dynamic "Tom Hanks" 2 8

./dynamic <start> <Depth>

In centarrus after making both files you just need to run sbatch bench_dynamic.sh and you will see the benchmark.

The benchmark tool produced the following:
Time to crawl: 0.428091s depth 2 w/blockq
Time to crawl: 1.78503s  depth 3 w/blockq
Time to crawl: 10.0818s  depth 4 w/blockq
Time to crawl: 3.2929s   depth 2 w/seq
Time to crawl: 72.0484s  depth 3 w/seq
Time to crawl: 480.112s  depth 4 w/seq
