from num2words import num2words

# Generate BitFunnel chunk data, with documents consisting of:
# {'one'}, {'one', 'two'}, {'one', 'two', 'three'}, ...

num_docs = 192

# This prints numbers one per line because of VC++'s string length limit.
# See https://github.com/BitFunnel/BitFunnel/issues/382. It would be
# possible to print up to the length limit, but this is simpler.
def print_numbers(num):
    for i in range(1, num+1):
        english_number = num2words(i).replace(' ','-') + "\\0"
        print('\"{}\"'.format(english_number))

def print_doc(id, name):
    print('\"{0:0>16x}\\0\"'.format(id))
    print('\"01\\0{}\\0\" \"{}\\0\\0\"'.format(name, id))
    print('\"00\\0\"')
    print_numbers(id)
    print('\"\\0\\0\"')

for i in range (1, num_docs+1):
    print_doc(i, 'Sequential')

print('//')
print('//')
print('//')

for i in range (1, num_docs+1):
    # print('<< "show rows {}" << std::endl'.format(num2words(i).replace(' ','-')))
    print('<< "verify one {}" << std::endl'.format(num2words(i).replace(' ','-').replace('-','\\\\-')))

