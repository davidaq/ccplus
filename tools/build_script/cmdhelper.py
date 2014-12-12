
source_file_filter         = '.*\.(mm?|c(pp)?)'

cxx_cmd       = 'g++ -c %src% -o %dst% -MMD'
c_cmd         = 'gcc -c %src% -o %dst% -MMD'

def parselist(list):
    for item in list:
        yield repr(item)
