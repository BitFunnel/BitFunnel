import subprocess

# Get history of linecount by date.

repo_dir = '/Users/visualstudio/dev/bf-count-lines'

def get_num_lines():
    # find_cmd = 'find inc src test tools -name "*.cpp" -o -name "*.h" | xargs wc'.split()
    find_cmd = 'find inc src test tools -name "*.cpp" -o -name "*.h" | grep -v Data | xargs wc'
    p_find = subprocess.Popen(find_cmd, cwd=repo_dir, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    find_out = p_find.stdout.read()
    num_lines = find_out.split()[-4].decode("utf-8")
    return num_lines

def checkout(sha):
    checkout_cmd = ['git', 'checkout', sha]
    p_checkout = subprocess.Popen(checkout_cmd, cwd=repo_dir,stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p_checkout.wait()


# Output from
# git log --date=format:'%Y-%m-%d %H:%M:%S' --format="%h,%ad"
git_log = [line.strip() for line in open('log-date')]
git_log.reverse()

print('{},{}'.format('date','wc'))
for line in git_log:
    sha, date = line.split(',')
    checkout(sha)
    wc = get_num_lines()
    print('{},{}'.format(date, wc))
