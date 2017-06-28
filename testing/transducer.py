import sys, re

transitions = {
    (0,'^') : 1,
    (1,'t') : 2,
    (2,'a') : 3,
    (3,'k') : 4,
    (4,'e') : 5,
    (5,'<vblex>') : 6,
    (6,'<ANY_TAG>') : 7,
    (7,'<ANY_TAG>') : 7,
    (7,'$'): 8,
    (8,' ') : 9,
    (9,'^') : 10,
    (10,'&') : 11,
    (11,'&') : 11,
    (11,'<det>') : 12,
    (11,'<adj>') : 13,
    (11,'<n>') : 14,
    (11,'<prn>') : 15,
    (12,'<ANY_TAG>') : 16,
    (13,'<ANY_TAG>') : 16,
    (14,'<ANY_TAG>') : 16,
    (15,'<ANY_TAG>') : 16,
    (16,'<ANY_TAG>') : 16,
    (16,'$') : 17,
    (17,' ') : 18,
    (18,'^') : 19, #?
    (19,'&') : 11,
    (19,'o') : 20,
    (20,'u') : 21,
    (21,'t') : 22,
    (22,'<adv>') : 23,
    (23,'<pr>') : 24,
    (23,'$') : 25,
    (24,'$') : 25,
    (25,'\n') : 26
}

states = {
    0 : '^',
    1 : 't',
    2 : 'a',
    3 : 'k',
    4 : 'e',
    5 : '<vblex>',
    6 : '<ANY_TAG>', #
    7 : '<ANY_TAG>', #the second one
    8 : '$',
    9 : ' ',
    10 : '^',
    11 : '&', #'ANY_CHAR', #
    12 : '<det>',
    13 : '<adj>',
    14 : '<n>',
    15 : '<prn>',
    16 : '<ANY_TAG>',
    17 : '$',
    18 : ' ',
    19 : '^',
    20 : 'o',
    21 : 'u',
    22 : 't',
    23 : '<adv>',
    24 : '<pr>',
    25 : '$',
    26 : '\n',
}

def next_token(first_tag_passed, in_lemma, in_take, in_out):
    token = sys.stdin.read(1)
    # print 'next_token' + token
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
    if in_lemma and not in_take and not in_out:
        # print in_lemma, in_take, in_out
        token = '&' #ANY_CHAR
    return token

def step(state, token): #token is at the next state
    next_state = transitions.get((state,token))

    if next_state == None:
        # print(str(state), str(current_state), str(token))
        print('error: (current_state,token) pair not found in transitions. ' + str(current_state) + str(token))
        exit(1)
    elif next_state == 25:
        print('successful termination')
        exit(0)

    print states[next_state]
    # print('inside step(): printing ' + states[next_state] + ' current state ' + str(next_state)) #prints the prev token
    return next_state #transitions.get((state,token)) #return the next state, or None if it doesn't exist

def main():
    print('input a string:')
    current_state = 0
    first_tag_passed = False
    in_lemma = False
    in_take = False
    in_out = False
    # token = next_token(first_tag_passed, in_lemma, in_take, in_out)

    while states.get(current_state) != None:
        token = next_token(first_tag_passed, in_lemma, in_take, in_out)
        # print 'before step(): token = ' + token + '   current_state = ' + str(current_state)
        next_state = step(current_state, token)

        first_tag_passed = next_state in [6, 7, 16, 12, 13, 14, 15, 16] #out not included
        in_lemma = next_state in [1, 2, 3, 4, 10, 11, 20, 21, 22] #take and out don't need to be included?
        in_take = next_state in [1, 2, 3, 4]
        # in_out = sys.stdin.read(4) == 'out<'
        in_out = next_state in [19, 20, 21] #-21  #should be: if peek(sys.stdin.read(3) == 'out<'

        current_state = next_state
        # print first_tag_passed, in_lemma, in_take, in_out
        # print 'token' + token
        # print str(current_state), str(states.get(current_state))
    print('error: current_state ' + state + ' not found in states')
    exit(0)

#^take<vblex><ANY_TAG>$ ^ccccc<det><pres><tag>$ ^cccc<n><sing>$ ^out<adv>$
#^take<vblex><pres><tag><moretag><moretag>$ ^the<det><sometag><anothertag>$ ^thing<n><sg><tag><Tag>$ ^out<adv>$
#^take<vblex><pres><tag1><tag2><tag3>$ ^thing<n><sg><tag><Tag>$ ^out<adv>$
#^take<vblex><pres>$ ^thing<n><sg><tag><Tag>$ ^out<adv>$

if __name__ == '__main__':
    main()