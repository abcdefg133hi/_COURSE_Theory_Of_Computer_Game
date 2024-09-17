#!/usr/bin/python3
from rich.segment import Segment
from rich.style import Style

from textual.app import App, ComposeResult
from textual.strip import Strip
from textual.widget import Widget
from textual.screen import Screen
from textual.reactive import reactive

from textual.widgets import Button, Header, Footer, Digits

import os

BOARD_SIZE=10

step_id=0
history_board=[[[-1 for _ in range(BOARD_SIZE)] for __ in range(BOARD_SIZE)]]
dice_seq=[0]*30

class EWNBoard(Widget):
    """Render an 10x10 EWNboard."""
    BORDER_TITLE = "EWN Board"
    COMPONENT_CLASSES = {
        "board--white-square",
        "board--gray-square",
    }

    def render_line(self, y: int) -> Strip:
        """Render a line of the widget. y is relative to the top of the widget."""
        row_index = y  # 1 line per row

        if row_index >= BOARD_SIZE+1:
            return Strip.blank(self.size.width)

        white = self.get_component_rich_style("board--white-square")
        gray = self.get_component_rich_style("board--gray-square")

        if row_index==0:
            segments = [Segment(" \\ ",style=Style(bold=True,bgcolor="#000000"))]
            segments += [Segment(f" {column} ",style=Style(bold=True,color="#FFFFFF",bgcolor="#000000"))
            for column in range(BOARD_SIZE)]
        else:
            segments = [Segment(f" {row_index-1} ",style=Style(bold=True,color="#FFFFFF",bgcolor="#000000"))]
            global history_board,step_id
            for column in range(BOARD_SIZE):
                if (history_board[step_id][row_index-1][column]!=-1):
                    segment=Segment(f" {history_board[step_id][row_index-1][column]} ", Style(bold=True,color="#FFFFFF",bgcolor="#00ADFC"))
                elif (column==3 and row_index==4):
                    segment=Segment(" X ",style=Style(bold=True,color="#FFFFFF",bgcolor="#000000"))
                elif (column + row_index) % 2:
                    segment=Segment("   ", gray)
                else:
                    segment=Segment("   ", white)
                segments.append(segment)
        strip = Strip(segments)
        return strip

class MessageBox(Widget):
    BORDER_TITLE = "Message Box"

    msgstr = reactive("")

    def render(self) -> str:
        return self.msgstr

class DiceIndicator(Widget):
    BORDER_TITLE = "Dice Sequence"
    def render_line(self, y: int) -> Strip:
        row_index = y  # 1 line per row
        if row_index >= 1:
            return Strip.blank(self.size.width)
        segments = [Segment("  ",style=Style(bold=True,bgcolor="#000000"))]
        global step_id,dice_seq
        for column in range(30):
            seq_index=column
            if seq_index==step_id:
                segment=Segment(f"{dice_seq[seq_index]}",style=Style(bold=True,color="#FFFFFF",bgcolor="#EE0000"))
            else:
                segment=Segment(f"{dice_seq[seq_index]}",style=Style(bold=True,color="#FFFFFF",bgcolor="#000000"))
            segments.append(segment)
        segments.append(Segment(" ",style=Style(bold=True,bgcolor="#000000")))
        strip = Strip(segments)
        return strip

class GoalIndicator(Digits):
    BORDER_TITLE = "Goal Piece"

class HW1_Visual_UI(App):
    CSS = """
        Screen {
            layout: grid;
            grid-size: 53 25;
            grid-rows: 1;
            grid-columns: 1;
            grid-gutter: 0;
            background: #000000;
        }

        EWNBoard{
            row-span: 13;
            column-span: 35;
            border: round #888888;
            border-title-color: #888888;
            background: #000000;
        }
        EWNBoard .board--white-square {
            background: #EEEEEE;
        }
        EWNBoard .board--gray-square {
            background: #D0D0D0;
        }

        DiceIndicator {
            row-span: 3;
            column-span: 35;
            border: round #888888;
            border-title-color: #888888;
            background: #000000;
        }

        GoalIndicator{
            row-span: 5;
            column-span: 18;
            border: round #888888;
            border-title-color: #888888;
            background: #000000;
            margin: 0;
        }

        MessageBox{
            row-span: 11;
            column-span: 18;
            border: round #888888;
            border-title-color: #888888;
            background: #000000;
        }

        #prev{
            row-span: 5;
            column-span: 10;
            margin: 1;
        }

        #next{
            row-span: 5;
            column-span: 10;
            margin: 1;
        }

        #LoadOutput{
            row-span: 5;
            column-span: 17;
            margin: 1;
        }

        #LoadInput{
            row-span: 5;
            column-span: 16;
            margin: 1;
        }
    """
    BINDINGS = [("q", "quit", "Quit")]
    # variables
    input_loaded=False
    legal_step_num=1

    """A simple app to show our widget."""
    def compose(self) -> ComposeResult:
        yield Header(icon="❌")
        yield EWNBoard()
        yield MessageBox()
        yield GoalIndicator()
        yield DiceIndicator()
        
        yield Button("Prev", id="prev", variant="success")
        yield Button("Next", id="next", variant="success")
        yield Button("Load INPUT", id="LoadInput", variant="success")
        yield Button("Load OUTPUT", id="LoadOutput", variant="success")

        yield Footer()
        
    def on_button_pressed(self, event: Button.Pressed) -> None:
        """Event handler called when a button is pressed."""
        global step_id
        if event.button.id == "prev":
            step_id=(step_id-1)%self.legal_step_num
            self.query_one(EWNBoard).refresh()
            self.query_one(DiceIndicator).refresh()
        elif event.button.id == "next":
            step_id=(step_id+1)%self.legal_step_num
            self.query_one(EWNBoard).refresh()
            self.query_one(DiceIndicator).refresh()
        elif event.button.id == "LoadInput":
            self.load_INPUT()
            self.query_one(EWNBoard).refresh()
            self.query_one(DiceIndicator).refresh()
            self.query_one(GoalIndicator).update(f"  {self.goal_piece}")
        elif event.button.id == "LoadOutput":
            self.load_OUTPUT()
            self.query_one(EWNBoard).refresh()
            self.query_one(DiceIndicator).refresh()

    def load_INPUT(self):
        # setup board
        if not os.path.isfile("INPUT"):
            self.query_one(MessageBox).msgstr = "Error: Lacking INPUT file!"
            return
        inputfile=open("INPUT","r")

        # init value
        global history_board,dice_seq,step_id
        history_board.clear()
        history_board.append([[-1 for _ in range(BOARD_SIZE)] for __ in range(BOARD_SIZE)])
        dice_seq=[0]*30
        self.legal_step_num=1
        step_id=0
        self.goal_piece=""

        line=inputfile.readline()
        self.goal_piece=int(line.strip())

        line=inputfile.readline()
        letters=line.strip().split()
        if len(letters)!=6:
            self.query_one(MessageBox).msgstr = "Error: Invalid input, piece number incorrect!"
            inputfile.close()
            return
        i=1
        for letter in letters:
            history_board[0][int(letter)//10][int(letter)%10]=i
            i+=1
        line=inputfile.readline().strip()
        if len(line)!=30:
            self.query_one(MessageBox).msgstr = f"Error: Invalid input, length of dice sequence incorrect!\n\nExpect:30,Get:{len(line)}"
            inputfile.close()
            return
        for i in range(30):
            dice_seq[i]=int(line[i])
        inputfile.close()
        self.input_loaded=True
        self.query_one(MessageBox).msgstr = "LOAD INPUT SUCCESS!"
    
    def load_OUTPUT(self):
        if not self.input_loaded:
            self.query_one(MessageBox).msgstr = "Error: You should load INPUT first!"
            return
        # setup board
        if not os.path.isfile("OUTPUT"):
            self.query_one(MessageBox).msgstr = "Error: Lacking OUTPUT file!"
            return
        outputfile=open("OUTPUT","r")

        # init value
        global step_id,history_board
        step_id=0
        self.legal_step_num=1
        del history_board[1:]
        line=outputfile.readline()
        runtime=float(line.strip())

        output_step_len=int(outputfile.readline())

        line=outputfile.readline()
        outputfile.close()
        letters=line.strip().split()
        if len(letters)!=2*output_step_len:
            self.query_one(MessageBox).msgstr = f"Error: Invalid input, step number incorrect!"
            return
        
        # run through steps
        for step_num in range(output_step_len):
            # start and end
            start=int(letters[2*step_num])
            start_x=start%10
            start_y=start//10
            end=int(letters[2*step_num+1])
            end_x=end%10
            end_y=end//10
            # illegal step
            if history_board[step_num][start_y][start_x]==-1:
                self.query_one(MessageBox).msgstr = f"Error: No piece to move at step#{step_num+1}:({start},{end})!\n\nBut you can still view the moves before."
                return
            # step is legal

            # copy the latest board
            history_board.append([[-1 for y in range(BOARD_SIZE)]for x in range(BOARD_SIZE)])
            for y in range(BOARD_SIZE):
                for x in range(BOARD_SIZE):
                    history_board[step_num+1][x][y]=history_board[step_num][x][y]
            # perform the step
            history_board[step_num+1][end_y][end_x]=history_board[step_num+1][start_y][start_x]
            history_board[step_num+1][start_y][start_x]=-1
            self.legal_step_num+=1

        self.query_one(MessageBox).msgstr = f"LOAD OUTPUT SUCCESS!\n\nYour process runs in {runtime} seconds.\n\nYour solution takes {self.legal_step_num-1} steps."

if __name__ == "__main__":
    app = HW1_Visual_UI()
    app.run()