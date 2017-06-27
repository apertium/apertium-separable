import sys, re

def next_token():
    token = sys.stdin.read(1)
    if token == '<':
        c = ''
        while c != '>':
            c = sys.stdin.read(1)
            token += c
    # print token
    return token

def main():
    states = ['^', 't', 'a', 'k', 'e', '<vblex>', '$', ' ', '^', 'ANY_CHAR', '<n>', '$', ' ', '^', 'o', 'u', 't', '$']
    #states = ['^', 't', 'a', 'k', 'e', '<vblex>', '<ANY_TAG>', '$', ' ', '^', 'ANY_CHAR', '<n>', '<ANY_TAG>', '$', ' ', '^', 'o', 'u', 't', '$']
    current_state = 0

    print('input a string:')
    c = next_token()

#^take<vblex><pres>$ ^the<det><def><sp>$ ^thing<n><sg>$ ^out<adv>$

    while c:
        if current_state == len(states): #end of input
            print('Yes')
            sys.exit(0)
        elif c == states[current_state]: #ok
            print c, current_state
            current_state += 1
        elif re.match('[a-zA-Z]', c):
            print c, current_state
        elif re.match('\<(.+?)\>', c):
            print c, current_state
        else: #error
            print('No')
            sys.exit(1)
        c = next_token()

if __name__ == '__main__':
    main()