import subprocess, os, timeit

# err type : WA(Wrong Answer),TE(Time Error)

def judge_output(input_string:str,output_string:str,correct_time:float,ans_len:int):
    output_lines=output_string.splitlines()
    output_time=float(output_lines[0])
    if (output_time<correct_time-0.1 or correct_time+0.1<output_time):
        # TimeError, which is 0 score
        return "TE"
    output_len=int(output_lines[1])
    if output_len>=ans_len+2:
        return "WA"
    output_steps=[int(number) for number in output_lines[2].rstrip().split()]
    if len(output_steps)!=2*output_len:
        return "WA"
    input_lines=input_string.splitlines()
    goal_piece=int(input_lines[0])-1
    piece_pos=[int(number) for number in input_lines[1].rstrip().split()]
    dice_seq=input_lines[2]
    board=[-1]*100
    for i in range(6):
        board[piece_pos[i]]=i
    for i in range(output_len):
        # check step start and step end are legal
        if output_steps[2*i+1]>=100 or output_steps[2*i+1]<0 or output_steps[2*i+1]>=100 or output_steps[2*i+1]<0:
            return "WA"
        if output_steps[2*i+1]==33 or output_steps[2*i]==output_steps[2*i+1]:
            return "WA"
        old_x=output_steps[2*i]%10
        old_y=output_steps[2*i]//10
        new_x=output_steps[2*i+1]%10
        new_y=output_steps[2*i+1]//10
        if abs(new_x-old_x)>1 or abs(new_y-old_y)>1:
            return "WA"
        piece_id=board[output_steps[2*i]]
        if piece_id==-1:
            return "WA"
        dice=int(dice_seq[i])-1
        if piece_id>dice:
            for j in range(dice,piece_id):
                if piece_pos[j]!=-1:
                    return "WA"
        elif piece_id<dice:
            for j in range(piece_id+1,dice+1):
                if piece_pos[j]!=-1:
                    return "WA"

        goal_piece_id=board[output_steps[2*i+1]]
        if goal_piece_id!=-1:
            piece_pos[goal_piece_id]=-1
        board[output_steps[2*i+1]]=board[output_steps[2*i]]
        board[output_steps[2*i]]=-1
    if board[0]==goal_piece:
        if output_len==ans_len+1:
            # SubOptimal
            return "SO"
        return "AC"
    else:
        return "WA"


problem_list=['1.1', '1.2', '1.3', '2.1', '2.2', '2.3', '3.1', '3.2', '3.3', 'bonus_sample1', 'bonus_sample2', 'bonus_sample3']
ProblemSetLines=[]

def write_back():
    global problem_list,ProblemSetLines
    for i in range(len(problem_list)):
        with open(f"public_testcases/{problem_list[i]}","w+") as f:
            f.writelines(ProblemSetLines[i])
        # prevent hack, delete the answers
    print("Finished! Write back the problem and answer!")

# prepare problem_set
problem_number=len(problem_list)
for filename in problem_list:
    with open(f"public_testcases/{filename}") as f:
        ProblemSetLines.append(f.readlines())
    # prevent hack, delete the answers
    os.remove(f"public_testcases/{filename}")
print("Finish loading the problem and answer!")

if not os.path.exists("./run"):
    print("./run doesn't exist! move your executable here!")
    write_back()
    exit(0)

total_score=0
#limit: 4GibiBytes of RAM
command="./run"

#command="ulimit -v 4194304 && ./run"
for i in range(problem_number):
    input_str=ProblemSetLines[i][0]+ProblemSetLines[i][1]+ProblemSetLines[i][2]
    print(f"{problem_list[i]} : ",end="")
    start=timeit.default_timer()
    try:
        result=subprocess.run(command,input=input_str,capture_output=True, text=True, shell=True, timeout=10)
        end=timeit.default_timer()
        runtime=end-start
        output_str=result.stdout
        judge_result=judge_output(input_str,output_str,runtime,int(ProblemSetLines[i][3]))
        print(judge_result)
        if judge_result=="AC":
            total_score+=1
    except:
    # handle timeout here
        print("RE/TLE")
print(f"***** AC : {total_score}/{problem_number} *****")
write_back()
