import sys, re

transitions = {
    (-1,'^') : 0,
    (0,'t') : 1,
    (1,'a') : 2,
    (2,'k') : 3,
    (3,'e') : 4,
    (4,'<vblex>') : 5,
    (5,'<ANY_TAG>') : 6,
    (6,'<ANY_TAG>') : 6,
    (6,'$') : 7,
    (7,' ') : 8,
    (8,'^') : 9,
    (9, '&') : 10,
    (10,'&') : 10,
    (10,'<det>') : 11,
    (10,'<adj>') : 12,
    (10,'<n>') : 13,
    (10,'<prn>') : 14,
    (11,'<ANY_TAG>') : 15,
    (12,'<ANY_TAG>') : 15,
    (13,'<ANY_TAG>') : 15,
    (14,'<ANY_TAG>') : 15,
    (15,'<ANY_TAG>') : 15,
    (15,'$') : 16,
    (16,' ') : 17,
    (17,'^') : 18, #?
    (18,'&') : 10,
    (18,'o') : 19,
    (19,'u') : 20,
    (20,'t') : 21,
    (21,'<adv>') : 22,
    (21,'<pr>') : 23,
    (22,'$') : 24,
    (23,'$') : 24,
    (24,'\n') : 25
}

states = {
    -1 : '^',
    0 : '^',
    1 : 't',
    2 : 'a',
    3 : 'k',
    4 : 'e',
    5 : '<vblex>',
    6 : '<ANY_TAG>', #
    7 : '$',
    8 : ' ',
    9 : '^',
    10 : '&', #'ANY_CHAR', #
    11 : '<det>',
    12 : '<adj>',
    13 : '<n>',
    14 : '<prn>',
    15 : '<ANY_TAG>',
    16 : '$',
    17 : ' ',
    18 : '^',
    19 : 'o',
    20 : 'u',
    21 : 't',
    22 : '<adv>',
    23 : '<pr>',
    24 : '$',
    25 : '\n',
}

def next_token(first_tag_passed, in_lemma, in_take, in_out):
    token = sys.stdin.read(1)
    if token == '<': #if in tag
        in_lemma = False
        c = ''
        while c != '>':
            c = sys.stdin.read(1)
            token += c
        if first_tag_passed:
            token = '<ANY_TAG>'
        # first_tag_passed = True
    # print in_lemma, in_take, in_out
    # print in_lemma, in_take, in_out
    if in_lemma and not in_take and not in_out:
        token = '&' #ANY_CHAR
    # if token == '^':
    #     in_lemma = True
    # if token == '^':
    #     c = sys.stdin.read(1)
    #     # while c != '<':
    #         token += c
    #         c = sys.stdin.read(1) #unread?
    # elif token == '\n':
    #     return ''
    return token

def step(state, token):
    print(states[state] + str(state))
    # if state == 6 or state == 15:
    #     token = '<ANY_TAG>'
    return transitions.get((state,token)) #return the next state, or None if it doesn't exist

def main():
    print('input a string:')
    current_state = -1
    first_tag_passed = False
    in_lemma = False
    in_take = False
    in_out = False
    token = next_token(first_tag_passed, in_lemma, in_take, in_out)

    while states.get(current_state) != None:
        next_state = step(current_state, token) #c is peeking; get next state
        if next_state == None:
            # print(str(state), str(current_state), str(token))
            print('error: (current_state,token) pair not found in transitions. ' + str(current_state) + str(token))
            exit(1)
        elif next_state == 25:
            print('successful termination')
            exit(0)

        first_tag_passed = next_state in [5, 6, 11, 12, 13, 14, 15]
        in_lemma = next_state in [1, 9, 10, 18]
        in_take = current_state in [0, 1, 2, 3, 4] #-4
        # in_out = sys.stdin.read(4) == 'out<'
        in_out = next_state in [19, 20, 21] #-21  #should be: if peek(sys.stdin.read(3) == 'out<'

        current_state = next_state
        token = next_token(first_tag_passed, in_lemma, in_take, in_out)
        # print str(current_state), str(states.get(current_state))
    print('error: current_state not found in states')
    exit(0)

#^take<vblex><ANY_TAG>$ ^ccccc<det><pres><tag>$ ^cccc<n><sing>$ ^out<adv>$
#^take<vblex><pres><tag><moretag><moretag>$ ^the<det><sometag><anothertag>$ ^thing<n><sg><tag><Tag>$ ^out<adv>$
#^take<vblex><pres><tag><moretag><moretag>$ ^thing<n><sg><tag><Tag>$ ^out<adv>$


if __name__ == '__main__':
    main()