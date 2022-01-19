import shutil

source_file = open('/data1/mtx/rmat23-631.mtx0' , 'r')
print(source_file.readline())
print(source_file.readline())
print(source_file.readline())
print(source_file.readline())



# this will truncate the file, so need to use a different file name:
#target_file = open('/data1/huge/weibo.el', 'w')

#shutil.copyfileobj(source_file, target_file)
source_file.close()