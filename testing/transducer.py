import sys, re

transitions = {
    (-1,'^') : 0,
    (0,'t') : 1,
    (1,'a') : 2,
    (2,'k') : 3,
    (3,'e') : 4,
    (4,'<vblex>') : 5,
    (5,'<ANY_TAG>') : 6,
    (6,'<ANY_TAG') : 6,
    (6,'$') : 7,
    (7,' ') : 8,
    (8,'^') : 9,
    (9,'c') : 10,
    (10,'c') : 10,
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
    (17,'^') : 18,
    (18,'c') : 10,
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
    -1 : '?',
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
    10 : 'c', #'ANY_CHAR', #
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
    25 : ''
}

def next_token():
    token = sys.stdin.read(1)
    if token == '<':
        c = ''
        while c != '>':
            c = sys.stdin.read(1)
            token += c
    # print token
    return token

def step(state, symbol):
    print(states[state])
    return transitions.get((state,symbol)) #return the next state, or None if it doesn't exist

print('input a string:')
current_state = -1
c = next_token()
while states.get(current_state) != None:
    current_state = step(current_state, c) #c is peeking; get next state
    if current_state == None:
        print('error: next state not found for ' (current_state,c) )
        exit(1)
    c = next_token()
exit(0)

#^take<vblex><ANY_TAG>$ ^ccccc<det><ANY_TAG><ANY_TAG>$ ^ccccc<n><ANY_TAG>$ ^out<adv>$
#^take<vblex><pres>$ ^the<det><def><sp>$ ^thing<n><sg>$ ^out<adv>$


# if __name__ == '__main__':
#     main()