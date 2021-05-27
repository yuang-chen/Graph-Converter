import shutil

source_file = open('/data/pagerank/raw_edgelist/wikipedia_link', 'r')
source_file.readline()
# this will truncate the file, so need to use a different file name:
target_file = open('/data/pagerank/raw_edgelist/wikipedia_link_edgelist', 'w')

shutil.copyfileobj(source_file, target_file)