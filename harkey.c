#include <windows.h>
#include<stdlib.h>
#include <stdint.h>

#define min(a, b) ((a < b) ? (a) : (b))
#define max(a, b) ((a > b) ? (a) : (b))

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;


static int running;
static f32 player_y;
static f32 player_y_speed;
static f32 AI_y;
static f32 AI_y_speed;
static f32 AI_speed;
static f32 ball_x;
static f32 ball_y;
static f32 ball_x_speed;
static f32 ball_y_speed;
static f32 ball_speed;
static int is_init;
static u32 player_score;
static u32 AI_score;
static f32 stage;
static u32 game_mode = 0;
void to_zero(u32 size, void *ptr) 
{
    u8 *byte = (u8*)ptr;
    while(size--) {
        *byte++ = 0; // 사이즈 영역 크기를 4 바이트씩 모두 0 으로 초기화
    }
}

struct {
    int ended_down;
} typedef button_state;

enum {
    w_button, // button_state 배열 0번 인덱스와 매핑
    s_button,  // button_state 배열 1번 인덱스와 매핑
};

struct {
    button_state buttons[2];
} typedef game_input;

struct {
    int   width;
    int   height;
    int   pitch;
    void *memory;
} typedef game_screen_info;

void draw_rect(game_screen_info *info, u32 color, f32 x, f32 y, f32 size_x, f32 size_y) 
{
    //전체적인 사이즈 6.5배 키워서 크게 보이게 함 
    size_x *= 6.5;
    size_y *= 6.5;
    x *= 6.5;
    y *= 6.5;

    // 윈도우의 중앙에 오게끔 재배치   
    x += info->width/2;
    y +=  info->height/2;
    
    //그리려는 X좌표 Y좌표 각각의 사이즈 만큼 양옆으로 여유 만들어 줌
    int min_x = (u32)(x - size_x);
    int min_y = (u32)(y - size_y);
    int max_x = (u32)(x + size_x);
    int max_y = (u32)(y + size_y);
    
    // 픽셀 별 한 바이트 별로 색깔 채워넣기
    min_x = max(min_x, 0); // 최소한 0
    min_y = max(min_y, 0); // 최소한 0
    max_x = min(max_x, info->width); // 최대는 윈도우 창 전체 너비
    max_y = min(max_y, info->height); // 최대는 윈도우 창 전체 높이
    //info-> memory는 전체 공간을 할당받고 시작 주소를 가리킴 min_x *4 byte만큼 앞으로 전진하고 min_y * 4byte * width만큼
    // 위로 올라가서 x,y 시작 주소를 지정  
    u8* row = (u8*)info->memory + min_x*sizeof(u32) + min_y*info->pitch; // info->pitch = 4byte * width
    for (int y = min_y; y < max_y; y++) {
        u32 *pixel = (u32*) row; // u32 사이즈인 컬러를 더하기 위해 row u32로 캐스팅
        for (int x = min_x; x < max_x; x++) {
            *pixel++ = color;
        }
        row += info->pitch; // 4byte * width만큼 위로 올린다.
    }
}

void draw_number(game_screen_info *info, u32 number, u32 color, f32 x, f32 y) 
{
    do 
    {
        //뒷자리부터 순서대로 숫자 그려나감
        //숫자별로 직사각형을 하나 씩 일일이 그린다.
        int one = number % 10;
        number /= 10;
        
        switch (one) 
        {
            case 0: {
                draw_rect(info, color, x - 2, y, 0.5, 4);
                draw_rect(info, color, x + 2, y, 0.5, 4);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
            } break;
            
            case 1: {
                draw_rect(info, color, x + 2, y, 0.5, 4.5);
            } break;
            
            case 2: {
                draw_rect(info, color, x - 2, y-2, 0.5, 2);
                draw_rect(info, color, x + 2, y+2, 0.5, 2);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x, y - 4., 2.5, 0.5);
            } break;
            
            case 3: {
                draw_rect(info, color, x + 2, y, 0.5, 4);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
            } break;
            
            case 4: {
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x + 2, y, 0.5, 4.5);
                draw_rect(info, color, x - 2, y+2.5, 0.5, 2);
            } break;
            
            case 5: {
                draw_rect(info, color, x + 2, y-2, 0.5, 2);
                draw_rect(info, color, x - 2, y+2, 0.5, 2);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
            } break;
            
            case 6: {
                draw_rect(info, color, x + 2, y-2, 0.5, 2);
                draw_rect(info, color, x - 2, y, 0.5, 4);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
            } break;
            
            case 7: {
                draw_rect(info, color, x + 2, y, 0.5, 4.5);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
            } break;
            
            case 8: {
                draw_rect(info, color, x - 2, y, 0.5, 4);
                draw_rect(info, color, x + 2, y, 0.5, 4);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
            } break;
            
            case 9: {
                draw_rect(info, color, x - 2, y+2, 0.5, 2);
                draw_rect(info, color, x + 2, y, 0.5, 4);
                draw_rect(info, color, x, y + 4, 2.5, 0.5);
                draw_rect(info, color, x, y, 2.5, 0.5);
                draw_rect(info, color, x, y - 4, 2.5, 0.5);
            } break;   
        }
        x -= 7; // 7만큼 뒤로 가서 출력
    } while (number > 0);
}

void game_update (game_input *input, game_screen_info *draw_info, HWND window) 
{
    
    f32 initial_ball_speed;
    f32 playing_field_x = 97;
    f32 playing_field_y = 52;
    f32 player_size_x = 2.5;
    f32 player_size_y = 10;
    f32 speed = 60; // 플레이어 바 스피드 
    if (game_mode == 1 )
    { // easy mode
        AI_speed = 35;
        initial_ball_speed = 70;
    }
    else if (game_mode == 2 )
    { // normal mode
        AI_speed = 40;
        initial_ball_speed = 80;
    }
    else if (game_mode == 3 )
    { // hard mode
        AI_speed = 45;
        initial_ball_speed = 90;
    }
    else if (game_mode == 4 )
    { // very hard mode
        AI_speed = 50;
        initial_ball_speed = 110;
    }

    if (!is_init) 
    {
        is_init = 1;
        ball_x = 0;
        ball_y = 0;
        ball_y_speed = 0;
        player_score = 0;
        stage = 0;
        AI_score = 0;
        player_y = 0;
        player_y_speed = 0;
        ball_x_speed = initial_ball_speed;
    }
    
    player_y_speed = 0; // 초기는 0
    
    if (input->buttons[w_button].ended_down) { // W 키를 입력했을 때
        if (player_y < playing_field_y - player_size_y) {
            player_y_speed = speed; 
        }
        //플레이어 바가 게임장 천장에 닿았다면 더 이상 진행하지 못하게
        if (player_y > playing_field_y - player_size_y){
            player_y = playing_field_y - player_size_y;
            player_y_speed = 0;
        }
    }
    if (input->buttons[s_button].ended_down) {
        if (player_y > -playing_field_y + player_size_y) {
            player_y_speed = -speed;
        }
        //AI 바가 게임장 바닥에 닿았다면 더 이상 진행하지 못하게
        if (player_y < -playing_field_y + player_size_y){
            player_y = -playing_field_y + player_size_y;
            player_y_speed = 0;
        }
    }
    // 시간의 흐름에 따른 플레이어 바 Y좌표 위치 변화
    player_y += player_y_speed * 0.001;

    
    AI_y_speed = (ball_y - AI_y) * 75; // 볼의 위치를 따라가게끔 초기화
    //AI_y_speed가 적어도 기존의 설정한 AI_speed만큼은 가지게 끔
    AI_y_speed = min(AI_y_speed, AI_speed);
    AI_y_speed = max(AI_y_speed, -AI_speed);
    AI_y += AI_y_speed * 0.001;  // 볼의 위치에 따른 AI 바 Y좌표 위치 변화
    //AI 바가 게임장 바닥에 닿았다면 더 이상 진행하지 못하게
    if (AI_y < -playing_field_y + player_size_y)
    {
        AI_y = -playing_field_y + player_size_y;
        AI_y_speed = 0;
    }
    //AI 바가 게임장 천장에 닿았다면 더 이상 진행하지 못하게
    if (AI_y > playing_field_y - player_size_y)
    {
        AI_y = playing_field_y - player_size_y;
        AI_y_speed = 0;
    }
    
    // 시간의 흐름에 따른 볼 x좌표 위치 변화
    ball_x += ball_x_speed * 0.001;

    // 시간의 흐름에 따라 볼의 속도를 계속해서 올린다
    if (ball_x_speed > 0) 
    {
        ball_x_speed += 5 * 0.001; 
    } 
    else 
    {
        ball_x_speed -= 5 * 0.001;
    }

    // 시간의 흐름에 따른 볼 y좌표 위치 변화
    ball_y += ball_y_speed * 0.001;
    
    if (ball_y > playing_field_y - 1) 
    {
        ball_y = playing_field_y - 1; // 볼이 윗 벽에 닿았을 경우 그것을 넘기지 못하게 방향 제어
        ball_y_speed *= -1;
    } 
    else if(ball_y < -playing_field_y + 1) 
    {
        ball_y = -playing_field_y + 1; // 볼이 아랫 벽에 닿았을 경우 그것을 넘기지 못하게 방향 제어
        ball_y_speed *= -1;
    }
    
    if (ball_x > 95 - player_size_x-1) //공이 ai 쪽 바에 근접했을 때
    {  
        //오른쪽 ai 바에 부딪혔을 때
        if ((ball_y >= (AI_y - player_size_y)) && (ball_y <= (AI_y + player_size_y))) 
        {
            ball_x_speed *= -1;
            ball_x = 92 - player_size_x - 1; 
            ball_y_speed = (ball_y - AI_y) + AI_y_speed;
        } 
        else if (ball_x >= playing_field_x - 1) // 공이 벽이랑 충돌했을 때 즉 점수 획득
        { 
            ball_x = 0;
            ball_y = 0;
            ball_y_speed = 0;
            stage++;
            ball_x_speed =  initial_ball_speed * (1+(stage*0.1)) ;// 사람이 이겼을 때 공을 ai에게 줌 즉 선공권을 줌
            player_score ++;
            player_y = 0;
            AI_y = 0;
        }
    } 
    else if (ball_x < -95 + player_size_x + 1)  //공이 사람 쪽 바에 근접했을 때
    {
        //왼쪽 플레이어 바에 충돌했을 때
        if ((ball_y >= (player_y - player_size_y)) && (ball_y <= (player_y + player_size_y))) 
        {
            ball_x_speed *= -1;
            ball_x = -92 + player_size_x + 1;
            ball_y_speed = (ball_y - player_y) + player_y_speed;
        } else if (ball_x <= -playing_field_x + 1) // 공이 벽이랑 충돌했을 때 즉 점수 획득
        { 
            ball_x = 0;
            ball_y = 0;
            ball_y_speed = 0;
            stage ++;
            ball_x_speed = -initial_ball_speed * (1+(stage*0.1)) ; // ai가 이겼을 때 공을 사람에게 줌 즉 선공권을 줌
            AI_score ++;
            player_y = 0;
            AI_y = 0;
        }
    }
    
    draw_rect(draw_info, 0x000000, 0, 0, draw_info->width, draw_info->height); // 이전에 그려졌던거 검정색으로 다 덮는다
    
    draw_rect(draw_info, 0xff0000, -92, player_y, player_size_x, player_size_y); // 플레이어 바 빨강색
    draw_rect(draw_info, 0x00ff00, 92, AI_y, player_size_x, player_size_y); // ai 바 초록색
    
    draw_rect(draw_info, 0xffff00, ball_x, ball_y, 1, 1); // 공 노란색
    draw_number(draw_info, (u32)stage, 0x808080, 0, 38); // 시간 회색
    draw_number(draw_info, player_score, 0xffffff, -20, 38); // 플레이어 점수 흰색
    draw_number(draw_info, AI_score, 0xffffff, 20, 38); // ai 점수 흰색
    draw_number(draw_info,game_mode,0xffffff,-60,38);

    if (ball_x_speed >= 0)
    { // 현재 공 속도 출력 흰색
        draw_number(draw_info, ball_x_speed, 0xffffff,60,38);
    } 
    else
    {
        draw_number(draw_info, -ball_x_speed, 0xffffff,60,38);
    }
    
}

struct {
    
    BITMAPINFO info;
    void      *memory;
    int        width;
    int        height;
    int        pitch;
    
} typedef window_screen_info;

struct {
    int width;
    int height;
} typedef window_size;

static window_screen_info global_window_screen_info;

// 윈도우 창의 너비와 높이 구한다.
window_size get_window_size(HWND window) 
{
    
    RECT client_rect;
    GetClientRect(window, &client_rect); // 윈도우 화면 좌표 값을 구하기 위한 함수 호출
    window_size size;
    size.width  = client_rect.right - client_rect.left; // 너비 오른쪽으로 증가
    size.height = client_rect.bottom - client_rect.top; // 높이 수학적인 상식과 다르게 y축이 아래 방향으로 향하기에 바텀에서 빼줘야 함
    
    return size;
}

// 윈도우 정보 초기화
void window_screen_info_init(window_screen_info *info, window_size size) 
{
    free(info->memory) ; // 메모리 값 초기화

    info->width  = size.width;
    info->height = size.height;
    info->pitch = sizeof(u32) * size.width; // 픽셀 공간 확보 4바이트 * 너비
    
    info->info.bmiHeader.biSize = sizeof(info->info.bmiHeader);
    info->info.bmiHeader.biWidth = info->width;
    info->info.bmiHeader.biHeight = info->height;
    info->info.bmiHeader.biPlanes = 1;
    info->info.bmiHeader.biBitCount = 32; // 픽셀당 비트 수 색깔 2^32개
    info->info.bmiHeader.biCompression = BI_RGB; //압축은 안한다
    
    u32 area = (info->pitch * info->height);
    
    info->memory = (u32 *)malloc(area); // area 영역 만큼의 공간 확보
}

void draw_window(window_screen_info *info, HDC device_context) 
{
    StretchDIBits(device_context,
                  0, 0, info->width, info->height,
                  0, 0, info->width, info->height,
                  info->memory, &info->info, DIB_RGB_COLORS, SRCCOPY);
                  // rgb값을 포함해서 비트맵을 원본 그대로 출력
}

LRESULT CALLBACK main_window_callback(HWND   window, UINT   message,
                                      WPARAM w_param, LPARAM l_param) 
{  
    LRESULT result = 0;
    switch(message) 
    {
        case WM_SIZE: 
        {         
            window_size size = get_window_size(window); //윈도우창의 너비 높이 사이즈 구한다
            window_screen_info_init(&global_window_screen_info, size);// 윈도우 버퍼 설정            
        } break;
        
        case WM_DESTROY: 
        { // 윈도우 창이 제거 됐을 때 dc 핸들
            exit(0);
        } break;
        
        case WM_CLOSE: 
        {
            exit(0); // 윈도우 창이 종료되어야 할 때
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:     
        case WM_PAINT: 
        { // 윈도우 창을 다시 그려야 할 때
            
            PAINTSTRUCT paint_struct;
            HDC device_context = BeginPaint(window, &paint_struct); //dc 핸들 발급 및 무효화 영역 해제
            
            draw_window(&global_window_screen_info, device_context); // 윈도우 창 그리기 함수
            
            EndPaint(window, &paint_struct); // 그리기 종료 무효화 영역 유효화 영역으로 갱신
            
        } break;
        
        default: 
        {
            result = DefWindowProcA(window, message, w_param, l_param); // 이외의 메시지들 운영체제가 자동처리
        }
    }
    return result;
}

void keyboard_input (game_input *keyboard_controller) 
{  
    MSG message;
    
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) //인풋이 있었는지 확인하고 메세지 큐에서 제거 getmessage를 보완한 형태
    {
        switch(message.message) {
            case WM_QUIT: 
            {
                exit(0); // 종료
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: 
            {  // 키보드의 입력이 끝났을 경우에만
                
                u32 vk_code = (u32)message.wParam;
                
                int is_down  = ((message.lParam & (1 << 31)) == 0); // 해당 키가 이벤트가 발생했을 때 눌려져 있었나?

                if (vk_code == 'W') 
                {
                    //w키로 바를 올린다
                    keyboard_controller->buttons[w_button].ended_down = is_down;
                } 
                else if (vk_code == 'S') 
                {
                    //s키로 바를 내린다
                    keyboard_controller->buttons[s_button].ended_down = is_down;
                } 
                
                else if (vk_code == VK_ESCAPE) 
                {
                    exit(0);              //esc키 종료
                } 

            } break;
            default: 
            {
                TranslateMessage(&message);  //키보드 입력 메세지 문자메시지로 큐에 저장
                DispatchMessage(&message); // 문자로 변환된 메시지 콜백 함수로 전송
            }
        }
    }
}

void choosing_level(HWND window)
{
    HDC device_context= GetDC(window);
    TextOut(device_context,100,40,"choose your level",17);
    TextOut(device_context,100,60,"push 1 button is easy mode",26);
    TextOut(device_context,100,80,"push 2 button is noraml mode",28);
    TextOut(device_context,100,100,"push 3 button is hard mode",26);
    TextOut(device_context,100,120,"push 4 button is very hard mode",31);
        
    MSG message;
    
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) //인풋이 있었는지 확인하고 메세지 큐에서 제거 getmessage를 보완한 형태
    { 
        switch(message.message) 
        {
            case WM_QUIT: 
            {
                exit(0);   // 종료
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: 
            {  // 키보드의 입력이 끝났을 경우에만
                
                u32 vk_code = (u32)message.wParam;
      
                if (vk_code == '1') //EASY MODE
                {
                    game_mode = 1;
                } 
                else if (vk_code == '2') //NORMAL MODE
                {
                    game_mode = 2;
                } 
                else if (vk_code == '3') //HARD MODE
                {
                    game_mode = 3;
                } 
                else if (vk_code == '4') // VERY HARD MODE
                {
                    game_mode = 4;
                } 
                else if (vk_code == VK_ESCAPE) 
                {
                    exit(0);               //esc키 종료
                } 

            } break;
            default: 
            {
                
                TranslateMessage(&message);  //키보드 입력 메세지 문자메시지로 큐에 저장
                DispatchMessage(&message); // 문자로 변환된 메시지 콜백 함수로 전송
            }

        }
    }
        ReleaseDC(window, device_context);
    return ;
}

void judge_win_and_replay(int who, HWND window)
{
    HDC device_context = GetDC(window);
    if(who == 1)
    {
        TextOut(device_context,600,300,"you win",7);
    }
    else if (who == 2)
    {
        TextOut(device_context,600,300,"AI win",6);
    }
    TextOut(device_context,600,320,"if you want replay push button and select mode",46);
    TextOut(device_context,600,340,"choose your level",17);
    TextOut(device_context,600,360,"push 1 button is easy mode",26);
    TextOut(device_context,600,380,"push 2 button is noraml mode",28);
    TextOut(device_context,600,400,"push 3 button is hard mode",26);
    TextOut(device_context,600,420,"push 4 button is very hard mode",31);
    TextOut(device_context,600,440,"you don't want replay push esc button",37);
    ReleaseDC(window, device_context);
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) //인풋이 있었는지 확인하고 메세지 큐에서 제거 getmessage를 보완한 형태
    {
        
        switch(message.message) 
        {
            case WM_QUIT: 
            {
                exit(0);   // 종료
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: 
            {  // 키보드의 입력이 끝났을 경우에만
                u32 vk_code = (u32)message.wParam;
      
                if (vk_code == '1')  //easy mode
                { 
                    game_mode = 1;
                } 
                else if (vk_code == '2')  //normal mode
                {
                    game_mode = 2;
                } 
                else if (vk_code == '3') //hard mode
                {
                    game_mode = 3;
                } 
                else if (vk_code == '4') //very hard mode
                {
                    game_mode = 4;
                } 
 
                else if (vk_code == VK_ESCAPE) 
                {
                    exit(0);               //esc키 종료
                } 

            } break;
            default: 
            {
                
                TranslateMessage(&message);  //키보드 입력 메세지 문자메시지로 큐에 저장
                DispatchMessage(&message); // 문자로 변환된 메시지 콜백 함수로 전송
            }

        }
    }
    return;
}

int CALLBACK WinMain(
HINSTANCE instance,
HINSTANCE prev_nstance,
LPSTR     command_line,
int       command_show) {
       
    WNDCLASS window_class;
    
    window_class.cbClsExtra = 0; // 클래스 추가 바이트 0
    window_class.cbWndExtra = 0; // 윈도우 추가 바이트 0
    window_class.hbrBackground = 0; // 브러시 핸들 사용 x
    window_class.lpszMenuName = 0; // 윈도우 클래스 이름 지정 x
    
    window_class.style = CS_HREDRAW|CS_VREDRAW; // 창의 너비 혹은 높이 변할때 wm_paint 메세지 운영체제가 호출 
                                                // 콜백함수에서 만든 wm_paint 실행
    window_class.lpfnWndProc = main_window_callback; // 직접 만들 콜백함수를 사용
    window_class.hInstance = instance; // 운영체제로 부터 할당받는 식별자
    window_class.hCursor   = 0; // 커서 사용 x
    SetCursor(0); // 화면에서 마우스 커서 제거
    window_class.hIcon = 0; // 윈도우 클래스 핸들 x
    
    RegisterClass(&window_class); // createwindowes 함수 사용 준비 윈도우 클래스 등록
    
    HWND window;

    window = CreateWindowEx(0, window_class.lpszClassName, "Harkey_game", WS_TILEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, instance, 0);
    // 윈도우창을 만든다 1280*700 사이즈의 크기로 한다.
    // x,y 좌표 운영체제에 따른 기본 값으로 한다. 
    // 타일형식 윈도우를 사용 윈도우를 생성하자마자 바로 보이도록 함
    if (window) {
        
        running = 1;
        
        game_input input;
        to_zero(sizeof(input),&input); // input 구조체 바이트 별로 0 으로 초기화
        
        while(game_mode ==0)
        {
            choosing_level(window); // 게임 모드를 정한다.
        }
        while (running) {

            game_screen_info game_screen_info;
            game_screen_info.memory = global_window_screen_info.memory;
            game_screen_info.width  = global_window_screen_info.width;
            game_screen_info.height = global_window_screen_info.height;
            game_screen_info.pitch  = global_window_screen_info.pitch;
            
            if (player_score == 10 ) // 최대 점수 10점
            {
                game_mode = 0;
                is_init = 0;
                to_zero(sizeof(input),&input); // input 정보 초기화
                while(game_mode ==0)
                {
                    judge_win_and_replay(1,window); // 승리 텍스트 출력하고 모드 재정의 하기 위한 함수
                }
            }
            else if(AI_score == 10) // 최대 점수 10점
            {
                game_mode = 0;
                is_init = 0;
                to_zero(sizeof(input),&input); // input 정보 초기화
                while(game_mode ==0)
                {
                    judge_win_and_replay(2,window); // 승리 텍스트 출력하고 모드 재정의 하기 위한 함수
                }               
            }

            keyboard_input(&input);
            
            HDC device_context = GetDC(window); // api로 dc 발급
            draw_window(&global_window_screen_info, device_context); // 윈도우창을 그린다.
            ReleaseDC(window, device_context); //dc 반납
      
            game_update(&input, &game_screen_info,window); // 게임 진행 및 그래픽 그리는 함수 출력
        } 
    }
}