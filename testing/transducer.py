#usage: python transducer.py testfile.txt

import sys

"""
noun phrase acceptor:
    n.*
    adj n.*
    adj.* n
    adj.* adj.* n.*
    det.* n.*
    det.* adj n.*
    prn*
    np*
"""

transitions = {
    #if current_state is -1 and the next token (the next thing to print)
    #is '^', then next_state() will print the next token,
    #which is located at state[0], and set current_state to 0
    (-1,'^') : 0,
    (0,'t') : 1,
    (1,'a') : 2,
    (2,'k') : 3,
    (3,'e') : 4,
    (4,'<vblex>') : 5,
    (5,'<ANY_TAG>') : 6,
    (6,'<ANY_TAG>') : 7,
    (6,'$') : 8,
    (7,'<ANY_TAG>') : 7,
    (7,'$'): 8,
    (8,' ') : 9,
    (9,'^') : 10,
    (10,'&') : 11,
    (11,'&') : 11,
    (11,'<n>') : 12, #if noun, there should be one or more add'l tags but no following words
    (11,'<adj>') : 13, #if adj, add'l tags are optional and should be followed by an n
    (11,'<det>') : 14,
    (11,'<prn>') : 15,
    (11,'<np>'): 15.5,
    # (12,'<ANY_TAG>') : 16, #### case: n*
    (12,'<ANY_TAG>') : 200,
    (200,'<ANY_TAG>') : 201,
    (200,'$') : 17,
    (201,'<ANY_TAG>') : 201,
    (201,'$') : 17,
    # (13,'<ANY_TAG>') : 13, #### case: adj(*) n*
    (13,'<ANY_TAG>') : 225,
    (13,'$') : 250,
    (225,'<ANY_TAG>') : 225,
    (225,'$') : 250, #followed by noun
    (250,' '):251,
    (251,'^'):252,
    (252,'&'):253,
    (253,'&'):253,
    (253,'<n>'):12,
    (253,'<adj>'):13,
    # (14,'<ANY_TAG>') : 16,
    (14,'<ANY_TAG>') : 275,
    (275,'<ANY_TAG>') : 276,
    (275,'$') : 250,
    (276,'<ANY_TAG>') : 276,
    (276,'$') : 250,
    # (15,'<ANY_TAG>') : 16, #prn.pers same as n
    (15,'<ANY_TAG>') : 200,
    #15.5 same as 15 and n
    (15.5,'<ANY_TAG>'): 200,
    (16,'<ANY_TAG>') : 100,
    (16, '$') : 17,
    (100,'<ANY_TAG>') : 100,
    (100,'$') : 17,
    (17,' ') : 18, #do not go to state 17 unless you are expecting 'out' to be the next word
    (18,'^') : 19, #?
    # (19,'&') : 11,
    (19,'o') : 20,
    (20,'u') : 21,
    (21,'t') : 22,
    (22,'<adv>') : 23,
    (22,'<pr>') : 24,
    (23,'$') : 25,
    (24,'$') : 25,
    (25,'') : 26,
    (25,' ') : 26,
    (25,'\n') : 26
}

#<ANY_TAG_A> is required
#<ANY_TAG_B> is optional
states = {
    -1 : '',
    0 : '^',
    1 : 't',
    2 : 'a',
    3 : 'k',
    4 : 'e',
    5 : '<vblex>',
    6 : '<ANY_TAG_A>', #secondary tag is necessary
    7 : '<ANY_TAG_B>', #third, fourth, fifth...tags are optional
    8 : '$',
    9 : ' ',
    10 : '^',
    11 : '&', #represents any character 'ANY_CHAR
    12 : '<n>',
    13 : '<adj>',
    14 : '<det>',
    15 : '<prn>',
    15.5 : '<np>',
    16 : '<ANY_TAG_A>',
    100: '<ANY_TAG_B>',
    200: '<ANY_TAG_A>',
    201: '<ANY_TAG_B>',
    225: '<ANY_TAG_B>',
    250: '$',
    251: ' ',
    252: '^',
    253: '&',
    275: '<ANY_TAG_A>',
    276: '<ANY_TAG_B>',
    17 : '$',
    18 : ' ',
    19 : '^',
    20 : 'o',
    21 : 'u',
    22 : 't',
    23 : '<adv>',
    24 : '<pr>',
    25 : '$',
    26 : '\n'
}

def next_token(file, subsequent_tag, in_lemma, in_take, in_out):
    original_token = file.read(1)
    modified_token = original_token
    if original_token == '<': #if in tag
        in_lemma = False
        c = ''
        while c != '>':
            c = file.read(1)
            original_token += c
            modified_token += c
        if subsequent_tag:
            modified_token = '<ANY_TAG>'
    if in_lemma and not in_take and not in_out:
        modified_token = '&' #ANY_CHAR
    return original_token, modified_token

def step(state, token): #token is at the next state
    next_state = transitions.get((state,token))
    output_token = states.get(next_state)
    return next_state, output_token #return the next state, or None if it doesn't exist

def main():
    f = open(sys.argv[1])
    # print('input a string:')
    # eol = True
    line_number = 0
    accepted = True
    while True: #while eol:
        # eol = False
        line = ''
        if accepted:
            line_number += 1
        current_state = -1

        subsequent_tag = False
        in_lemma = False
        in_take = False
        in_out = False

        while states.get(current_state) != None and current_state != 26:
            # print states.get(current_state) + str(current_state)
            original_token, modified_token = next_token(f, subsequent_tag, in_lemma, in_take, in_out)
            if current_state == -1 and modified_token == '':
                print('successfully reached end of file')
                exit(0)
            elif current_state == -1 and modified_token == '\n':
                accepted = True
                break
            elif modified_token == '\n': #not accepted and token == '\n':
                accepted = True
            next_state, output_token = step(current_state, modified_token)
            if output_token == None:
                break

            line += original_token #line += output_token

            subsequent_tag = next_state in [5, 6, 7, 12, 13, 14, 15, 15.5, 16, 100, 200, 201, 225, 275, 276] #every state that is a tag. secondary tags for 'out' not included because it only ever has one tag
            in_lemma = next_state in [1, 2, 3, 10, 11, 252, 253, 19, 20, 21, 22] #include 4? do not include 22?
            in_take = next_state in [1, 2, 3, 4]
            # print 'position: ' + str(f.tell())
            if next_state == 19:
                #in c: there is an istream::peek() function
                pos = f.tell() #store the current buffer position
                peek = f.read(4) #read in the next 4 chars
                f.seek(pos) #go back to the original position
                if peek == 'out<':
                    in_out = True
            #TODO: when transitions are finalized, check indices

            current_state = next_state #can't set this earlier, or else the following print statement doesn't work
        if current_state == 26:
            print str(line_number) + '   ' + line
            accepted = True
        else:
            # print('error: current_state ' + str(current_state) + ' not found in states')
            # exit(1)
            if accepted:
                print str(line_number) + '   string not accepted \n'
                accepted = False
                current_state = -1
                line_number += 1
        # eol = True

if __name__ == '__main__':
    main()