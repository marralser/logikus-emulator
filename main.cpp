#include <SDL2/SDL.h>
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

constexpr int SCALE = 2;
constexpr int W = 640, H = 365;
constexpr int BOARD_TOP_Y = 130;
constexpr int STRIPE_Y = 265;
constexpr int SWITCH_Y = 295;
constexpr int MAXS = 19, MAXZ = 10;
constexpr int BREITE = 29, HOEHE = 24;

struct Contact { int s=-1,z=-1,p=0; bool valid() const { return s>=0 && z>=0 && p>=1 && p<=3; } };
struct Pt { int x=0,y=0; };
struct Wire { Contact a,b; std::vector<Pt> pts; };
struct Node { std::array<Contact,4> z; bool strom=false; };

SDL_Renderer* R=nullptr;
std::array<std::array<Node,11>,20> plan;
std::vector<Wire> wires;
std::array<bool,10> switches{};
std::array<std::string,10> lampLabels = {"L0","L1","L2","L3","L4","L5","L6","L7","L8","L9"};
bool redMode=false, dirty=true, buttonDown=false;
int editingLamp = -1;
std::string editBuffer;
std::string statusMsg = "n=new  s=save  l=load  r=red  q=quit  Click lamp label to edit";
Contact activeStart; std::vector<Pt> activePts;
int mx=0,my=0;
std::array<std::string,5> buf = {" "," ","    0     1       2      3      4      5       6      7      8      9"," "," "};

void setc(int r,int g,int b){ SDL_SetRenderDrawColor(R,r,g,b,255); }
void line(int x1,int y1,int x2,int y2){ SDL_RenderDrawLine(R,x1*SCALE,y1*SCALE,x2*SCALE,y2*SCALE); }
void fill(int x1,int y1,int x2,int y2){ SDL_Rect r{x1*SCALE,y1*SCALE,(x2-x1+1)*SCALE,(y2-y1+1)*SCALE}; SDL_RenderFillRect(R,&r); }
void box(int x1,int y1,int x2,int y2){ SDL_Rect r{x1*SCALE,y1*SCALE,(x2-x1)*SCALE,(y2-y1)*SCALE}; SDL_RenderDrawRect(R,&r); }
void dot(int x,int y,int rad=2){ fill(x-rad,y-rad,x+rad,y+rad); }

static const std::array<std::string,96> font = []{
    std::array<std::string,96> f{}; for(auto &s:f) s="00000000000000000000000000000000000";
    auto put=[&](char c,std::initializer_list<const char*> rows){ std::string s; for(auto r:rows)s+=r; f[c-32]=s; };
    put('0',{"111","101","101","101","101","101","111"}); put('1',{"010","110","010","010","010","010","111"});
    put('2',{"111","001","001","111","100","100","111"}); put('3',{"111","001","001","111","001","001","111"});
    put('4',{"101","101","101","111","001","001","001"}); put('5',{"111","100","100","111","001","001","111"});
    put('6',{"111","100","100","111","101","101","111"}); put('7',{"111","001","001","010","010","010","010"});
    put('8',{"111","101","101","111","101","101","111"}); put('9',{"111","101","101","111","001","001","111"});
    put('A',{"010","101","101","111","101","101","101"}); put('B',{"110","101","101","110","101","101","110"});
    put('C',{"111","100","100","100","100","100","111"}); put('D',{"110","101","101","101","101","101","110"});
    put('E',{"111","100","100","110","100","100","111"}); put('F',{"111","100","100","110","100","100","100"});
    put('G',{"111","100","100","101","101","101","111"}); put('H',{"101","101","101","111","101","101","101"});
    put('I',{"111","010","010","010","010","010","111"}); put('J',{"001","001","001","001","101","101","111"});
    put('K',{"101","101","110","100","110","101","101"}); put('L',{"100","100","100","100","100","100","111"});
    put('M',{"101","111","111","101","101","101","101"}); put('N',{"101","111","111","111","101","101","101"});
    put('O',{"111","101","101","101","101","101","111"}); put('P',{"111","101","101","111","100","100","100"});
    put('Q',{"111","101","101","101","111","001","001"}); put('R',{"110","101","101","110","110","101","101"});
    put('S',{"111","100","100","111","001","001","111"}); put('T',{"111","010","010","010","010","010","010"});
    put('U',{"101","101","101","101","101","101","111"}); put('V',{"101","101","101","101","101","101","010"});
    put('W',{"101","101","101","101","111","111","101"}); put('X',{"101","101","010","010","010","101","101"});
    put('Y',{"101","101","101","010","010","010","010"}); put('Z',{"111","001","001","010","100","100","111"});
    put('a',{"000","000","110","001","111","101","111"}); put('b',{"100","100","110","101","101","101","110"});
    put('c',{"000","000","111","100","100","100","111"}); put('d',{"001","001","011","101","101","101","011"});
    put('e',{"000","000","111","101","111","100","111"}); put('f',{"011","100","100","110","100","100","100"});
    put('g',{"000","000","111","101","111","001","111"}); put('h',{"100","100","110","101","101","101","101"});
    put('i',{"010","000","110","010","010","010","111"}); put('j',{"001","000","001","001","001","101","111"});
    put('k',{"100","100","101","110","100","110","101"}); put('l',{"110","010","010","010","010","010","111"});
    put('m',{"000","000","111","111","101","101","101"}); put('n',{"000","000","110","101","101","101","101"});
    put('o',{"000","000","111","101","101","101","111"}); put('p',{"000","000","111","101","111","100","100"});
    put('q',{"000","000","111","101","111","001","001"}); put('r',{"000","000","101","110","100","100","100"});
    put('s',{"000","000","111","100","111","001","111"}); put('t',{"100","100","110","100","100","100","011"});
    put('u',{"000","000","101","101","101","101","111"}); put('v',{"000","000","101","101","101","101","010"});
    put('w',{"000","000","101","101","101","111","101"}); put('x',{"000","000","101","010","010","010","101"});
    put('y',{"000","000","101","101","111","001","111"}); put('z',{"000","000","111","001","010","100","111"});
    put('-',{"000","000","000","111","000","000","000"}); put(':',{"000","010","010","000","010","010","000"}); put('.',{"000","000","000","000","000","110","110"});
    return f;
}();
void text(int x,int y,const std::string& s,int sc=1){ for(char ch:s){ if(ch==' '){x+=4*sc; continue;} unsigned char c=ch; if(c<32||c>127){x+=4*sc;continue;} auto pat=font[c-32]; for(int row=0;row<7;row++) for(int col=0;col<3;col++) if(pat[row*3+col]=='1') fill(x+col*sc,y+row*sc,x+(col+1)*sc-1,y+(row+1)*sc-1); x+=4*sc; } }

Contact holeAt(int x,int y){
    Contact c; bool schrift=false; int z=(y-BOARD_TOP_Y)/13, s=(x-73)/29, zm=(y-BOARD_TOP_Y)%13; if(zm<0) zm+=13;
    if(z&1) zm++; int pm=1+zm/4; if(zm%4==0) pm=0; int sm=(x-73)%58; if(sm<0) sm+=58;
    if(sm>13) s++; if((sm<1)||((sm>7&&sm<22)||sm>28)){pm=0;s=-1;} if(x<73||y<BOARD_TOP_Y+1){s=-1;pm=0;} if(y>BOARD_TOP_Y+128){s=-1;pm=0;} if(z>9) z=-1;
    if(y>48 && y<52){ z=10; s=(x-8)/59; sm=(x-8)%59; if(sm<0) sm+=59; pm=1+sm/6; if(sm%6==0) pm=0; }
    if(x<36){ z=10; sm=(x-8)%59; if(sm<0) sm+=59; pm=1+sm/6; if(sm%6==0) pm=0; if(y>BOARD_TOP_Y-2&&y<BOARD_TOP_Y+2) s=11; else if(y>BOARD_TOP_Y+23&&y<BOARD_TOP_Y+27) s=12; else if(y>48&&y<52) s=0; else pm=0; }
    if(pm==0||pm>3||z>10||s<0||s>MAXS){ return {}; } c.s=s; c.z=z; c.p=pm; return c;
}
Pt holePos(Contact c){ int x=c.s*29+73+4; if(c.s&1) x-=8; int y=c.z*13+BOARD_TOP_Y-2+4*c.p; if(c.z&1) y--; if(c.z==10){ x=c.s*59+7+6*c.p; y=50; if(c.s==11){x=7+6*c.p;y=BOARD_TOP_Y;} if(c.s==12){x=7+6*c.p;y=BOARD_TOP_Y+25;} } return {x,y}; }
bool same(Contact a,Contact b){ return a.s==b.s&&a.z==b.z&&a.p==b.p; }

void resetPlan(bool clearWires){
    for(auto &col:plan) for(auto &n:col){ n.z={}; n.strom=false; }
    for(int i=0;i<20;i++) for(int j=0;j<10;j++) if(j&1){ if(i&1) plan[i][j].z[0]={i-1,j,1}; else plan[i][j].z[0]={i+1,j,1}; }
    for(int i=0;i<10;i++) switches[i]=false;
    if(clearWires) wires.clear(); dirty=true;
}
void rebuildWireLinks(){ for(auto &col:plan) for(auto &n:col) for(int p=1;p<=3;p++) n.z[p]={}; for(auto &w:wires){ plan[w.a.s][w.a.z].z[w.a.p]=w.b; plan[w.b.s][w.b.z].z[w.b.p]=w.a; } }
void updateSwitchLinks(){ for(int sw=0;sw<10;sw++){ int i=sw*2; for(int j=0;j<10;j++){ bool connect = switches[sw] ? !(j&1) : (j&1); plan[i][j].z[0]= connect?Contact{i+1,j,1}:Contact{}; plan[i+1][j].z[0]= connect?Contact{i,j,1}:Contact{}; } } if(buttonDown){ plan[11][10].z[0]={12,10,1}; plan[12][10].z[0]={11,10,1}; } else { plan[11][10].z[0]={}; plan[12][10].z[0]={}; } }
void simulate(){ for(auto &col:plan) for(auto &n:col) n.strom=false; rebuildWireLinks(); updateSwitchLinks(); std::vector<Contact> st{{0,10,1},{0,10,2},{0,10,3}}; while(!st.empty()){ Contact c=st.back(); st.pop_back(); if(!c.valid()) continue; auto &n=plan[c.s][c.z]; if(n.strom) continue; n.strom=true; for(int p=0;p<=3;p++){ Contact nx=n.z[p]; if(nx.valid()&&!plan[nx.s][nx.z].strom) st.push_back(nx); } } dirty=false; }
bool occupied(Contact c){ if(!c.valid()) return false; return plan[c.s][c.z].z[c.p].valid(); }
void removeWireAt(Contact c){ for(size_t i=0;i<wires.size();++i){ if(same(wires[i].a,c)||same(wires[i].b,c)){ wires.erase(wires.begin()+i); dirty=true; return; } } }

void drawBox(int x,int y){ int x1=73+x*2*BREITE,y1=BOARD_TOP_Y+y*(HOEHE+2),x2=x1+BREITE,y2=y1+HOEHE; setc(80,80,80); line(x1-1,y2,x1-1,y1); line(x1,y2,x1,y1); line(x1,y1,x2,y1); setc(230,230,230); line(x2,y1,x2,y2); line(x2,y2,x1,y2); setc(180,180,180); line(x1+5,y1+HOEHE/2,x2-5,y1+HOEHE/2); setc(0,0,0); for(int j=y1+2;j<=y2-2;j+=4){ line(x1+3,j,x1+5,j); line(x2-5,j,x2-3,j); } }
void drawSwitch(int x,bool on){ int x1=73+x*2*BREITE+6,y1=SWITCH_Y,x2=x1+BREITE-6,y2=SWITCH_Y+20; setc(210,210,210); fill(x1-3,y1-13,x2+2,y2); if(on){ setc(0,0,0); fill(x1+7,y1,x2-7,y2); setc(200,40,40); fill(x1,y1-13,x2,y2-13); } else { setc(200,40,40); fill(x1,y1,x2,y2); setc(0,0,0); fill(x1+7,y1-13,x2-7,y1-1); } setc(40,40,40); box(x1,on?y1-13:y1,x2,on?y2-13:y2); }
void drawLamp(int x){
    int x1=48+x*59,y1=2,x2=x1+57,y2=47;
    bool on=plan[x+1][10].strom;
    if(on) setc(255-std::min(80,x*8),200-std::min(70,x*6),120); else setc(90,45,20);
    fill(x1,y1,x2,y2);
    setc(0,0,0);
    box(x1,y1,x2,y2);

    std::string label = (editingLamp==x) ? (editBuffer + "_") : lampLabels[x];
    if(label.size() > 9) label = label.substr(0,9);
    int tx = x1 + std::max(0, (57 - int(label.size())*4) / 2);
    text(tx,68,label,1);

    if(editingLamp==x){
        setc(255,220,30);
        box(x1-1,65,x2+1,79);
    }
}
void drawBoard(){
    if(dirty) simulate();
    setc(210,210,210);
    SDL_RenderClear(R);

    for(int i=0;i<10;i++){
        drawLamp(i);
        for(int j=0;j<5;j++) drawBox(i,j);
        drawSwitch(i,switches[i]);
    }

    // T push button. The original red button is now yellow while held down.
    if(buttonDown) setc(255,220,30);
    else setc(200,40,40);
    fill(21,SWITCH_Y,44,SWITCH_Y+20);
    setc(40,40,40);
    box(21,SWITCH_Y,44,SWITCH_Y+20);

    setc(80,80,80);
    fill(3,STRIPE_Y,637,STRIPE_Y+10);
    setc(230,230,230);
    box(3,STRIPE_Y,637,STRIPE_Y+10);

    // Labels. These are deliberately outside the 256px Amiga board area now,
    // because the modern window has extra vertical space below the board.
    setc(0,0,0);
    text(14,284,"T",2);
    text(4,42,"+",2);
    text(4,123,"Ta",1);
    text(4,148,"Tb",1);
    text(278,92,"LOGIKUS",2);

    // Switch labels: place T0..T9 at the top-right of each switch.
    for(int i=0;i<10;i++){
        int x1 = 73 + i * 2 * BREITE + 6;
        text(x1 - 12, SWITCH_Y - 11, "T" + std::to_string(i), 1);
    }

    // Command/status line in the new free space below the board.
    text(8,340,statusMsg,1);

    for(auto &w:wires){
        bool hot=plan[w.a.s][w.a.z].strom||plan[w.b.s][w.b.z].strom;
        setc(hot&&redMode?220:20,hot&&redMode?30:20,hot&&redMode?30:20);
        for(size_t i=1;i<w.pts.size();++i){
            line(w.pts[i-1].x,w.pts[i-1].y,w.pts[i].x,w.pts[i].y);
            line(w.pts[i-1].x+1,w.pts[i-1].y,w.pts[i].x+1,w.pts[i].y);
        }
    }
    if(activeStart.valid()){
        setc(20,20,20);
        Pt last=activePts.back();
        line(last.x,last.y,mx,my);
    }
    setc(0,0,0);
    for(int s=0;s<=12;s++){
        for(int p=1;p<=3;p++){
            Contact c{s,10,p};
            Pt q=holePos(c);
            if((s<=10&&q.x<640)||(s==11||s==12)) dot(q.x,q.y,1);
        }
    }
}

int switchAt(int x,int y){ if(y<SWITCH_Y-13||y>SWITCH_Y+20) return -1; if(x<79) return -2; int s=(x-79)/58; return (s>=0&&s<10)?s:-1; }
int lampLabelAt(int x,int y){
    if(y < 64 || y > 82) return -1;
    for(int i=0;i<10;i++){
        int x1 = 48 + i*59;
        int x2 = x1 + 57;
        if(x >= x1 && x <= x2) return i;
    }
    return -1;
}
void beginLampEdit(int i){
    if(i<0 || i>=10) return;
    editingLamp = i;
    editBuffer = lampLabels[i];
    SDL_StartTextInput();
    statusMsg = "Editing lamp label " + std::to_string(i) + ": type text, Enter=keep, Esc=cancel";
}
void finishLampEdit(bool keep){
    if(editingLamp>=0){
        if(keep){
            if(editBuffer.empty()) editBuffer = "L" + std::to_string(editingLamp);
            if(editBuffer.size() > 9) editBuffer.resize(9);
            lampLabels[editingLamp] = editBuffer;
            statusMsg = "Lamp label saved";
        } else {
            statusMsg = "Lamp label edit cancelled";
        }
    }
    editingLamp = -1;
    editBuffer.clear();
    SDL_StopTextInput();
}
void saveFile(const std::string& fn){
    std::ofstream o(fn);
    if(!o){ statusMsg="Save failed: "+fn; return; }
    o<<"LOGIKUS-LINUX 2\n"<<redMode<<"\n";
    for(bool b:switches)o<<b<<' ';
    o<<"\nLABELS 10\n";
    for(auto &label:lampLabels) o<<label<<"\n";
    o<<wires.size()<<"\n";
    for(auto&w:wires){
        o<<w.a.s<<' '<<w.a.z<<' '<<w.a.p<<' '<<w.b.s<<' '<<w.b.z<<' '<<w.b.p<<' '<<w.pts.size();
        for(auto&p:w.pts)o<<' '<<p.x<<' '<<p.y;
        o<<"\n";
    }
    statusMsg="Saved to "+fn;
}
void loadFile(const std::string& fn){
    std::ifstream in(fn);
    if(!in){ statusMsg="Load failed: "+fn; return; }
    std::string magic, version;
    in>>magic>>version;
    if(magic!="LOGIKUS-LINUX"){ statusMsg="Not a Logikus save file"; return; }
    resetPlan(true);
    in>>redMode;
    for(bool &sw:switches) in>>sw;

    std::string token;
    in >> token;
    if(token=="LABELS"){
        int count=0;
        in >> count;
        std::string dummy;
        std::getline(in,dummy); // consume end of line
        for(int i=0;i<count && i<10;i++){
            std::getline(in,lampLabels[i]);
            if(lampLabels[i].empty()) lampLabels[i] = "L" + std::to_string(i);
            if(lampLabels[i].size() > 9) lampLabels[i].resize(9);
        }
        for(int i=count;i<10;i++) lampLabels[i] = "L" + std::to_string(i);
        in >> token; // wire count follows the labels
    }

    size_t n=0;
    try { n = static_cast<size_t>(std::stoull(token)); }
    catch(...) { statusMsg="Save file is damaged"; return; }

    for(size_t k=0;k<n;k++){
        Wire w; size_t pc;
        in>>w.a.s>>w.a.z>>w.a.p>>w.b.s>>w.b.z>>w.b.p>>pc;
        for(size_t i=0;i<pc;i++){
            Pt p; in>>p.x>>p.y; w.pts.push_back(p);
        }
        if(w.a.valid()&&w.b.valid()&&w.pts.size()>=2) wires.push_back(w);
    }
    dirty=true;
    statusMsg="Loaded "+fn;
}


int main(int argc,char**argv){ resetPlan(true); if(argc>1) loadFile(argv[1]); if(SDL_Init(SDL_INIT_VIDEO)!=0){std::cerr<<SDL_GetError()<<"\n";return 1;} SDL_Window* win=SDL_CreateWindow("Logikus Linux",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,W*SCALE,H*SCALE,SDL_WINDOW_SHOWN); if(!win){std::cerr<<SDL_GetError()<<"\n";return 1;} R=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC); bool run=true; SDL_Event e; while(run){ while(SDL_PollEvent(&e)){ if(e.type==SDL_QUIT) run=false; if(e.type==SDL_MOUSEMOTION){ mx=e.motion.x/SCALE; my=e.motion.y/SCALE; }
            if(e.type==SDL_TEXTINPUT && editingLamp>=0){
                if(editBuffer.size() < 9) editBuffer += e.text.text;
            }
            if(e.type==SDL_KEYDOWN){
                SDL_Keycode k=e.key.keysym.sym;
                SDL_Scancode sc=e.key.keysym.scancode;
                if(editingLamp>=0){
                    if(k==SDLK_RETURN || k==SDLK_KP_ENTER) finishLampEdit(true);
                    else if(k==SDLK_ESCAPE) finishLampEdit(false);
                    else if(k==SDLK_BACKSPACE && !editBuffer.empty()) editBuffer.pop_back();
                    else if(k==SDLK_DELETE) editBuffer.clear();
                } else {
                    if(k==SDLK_q||k==SDLK_ESCAPE||sc==SDL_SCANCODE_Q) run=false;
                    else if(k==SDLK_n||sc==SDL_SCANCODE_N){ resetPlan(true); statusMsg="New board"; }
                    else if(k==SDLK_r||sc==SDL_SCANCODE_R){ redMode=!redMode; dirty=true; statusMsg=redMode?"Red live-wire mode on":"Red live-wire mode off"; }
                    else if(k==SDLK_s||sc==SDL_SCANCODE_S) saveFile("logikus.save");
                    else if(k==SDLK_l||sc==SDL_SCANCODE_L) loadFile("logikus.save");
                }
            }
            if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT){ mx=e.button.x/SCALE; my=e.button.y/SCALE; if(editingLamp>=0) finishLampEdit(true); int lamp=lampLabelAt(mx,my); if(lamp>=0){ beginLampEdit(lamp); } else { int sw=switchAt(mx,my); if(sw==-2){ buttonDown=true; dirty=true; } else if(sw>=0){ switches[sw]=!switches[sw]; dirty=true; } else { Contact c=holeAt(mx,my); if(c.valid()){ if(activeStart.valid()){ if(!same(c,activeStart)&&!occupied(c)){ Wire w; w.a=activeStart; w.b=c; w.pts=activePts; w.pts.push_back(holePos(c)); wires.push_back(w); dirty=true; } activeStart={}; activePts.clear(); } else { if(occupied(c)) removeWireAt(c); else { activeStart=c; activePts={holePos(c)}; } } } } } }
            if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_RIGHT){ mx=e.button.x/SCALE; my=e.button.y/SCALE; if(activeStart.valid()) activePts.push_back({mx,my}); else { Contact c=holeAt(mx,my); if(c.valid()) removeWireAt(c); } }
            if(e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT){ if(buttonDown){ buttonDown=false; dirty=true; } }
        } drawBoard(); SDL_RenderPresent(R); }
    SDL_DestroyRenderer(R); SDL_DestroyWindow(win); SDL_Quit(); return 0; }
