/*  Modular 8 steps Sequencer
    Copyright (C) 2020  Ernesto Gigliotti <ernestogigliotti@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/    

void track_init(unsigned char initTrack,unsigned char initScale,unsigned char initRootNote);
void track_playStep(int indexStep,int trackIndex);
void track_loop(void);
int track_nextScale(void);
void track_tick1ms(void);
int track_getCurrentTrack(void);
void track_silenceStep(int stepIndex, int trackIndex);
int track_nextTrack(void);
int track_getCurrentStepInTrack(void);
int track_getCurrentScale(void);

int track_nextProbRptMode(void);
int track_getCurrentProbRptMode(void);

int track_nextRootNote(void);
int track_getCurrentRootNote(void);

unsigned char track_prevEuclideanStep(void);
unsigned char track_nextEuclideanStep(void);
unsigned char track_getCurrentEuclideanStepsOn(void);
unsigned char* track_getEuclideanStepsState(void);

int track_getCurrentEuclideanOffset(void);
int track_nextEuclideanOffset(void);
int track_prevEuclideanOffset(void);
