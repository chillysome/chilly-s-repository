#pragma warning(disable:4996)
#include <stdio.h>
#include <graphics.h>  //图形库头文件
#include <time.h>
#include "Tetris.h"
#include <conio.h>
#include <stdlib.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")//加载库文件
#include "Vfw.h"
#pragma comment (lib, "Vfw32.lib")
#define ALLIMAGE 148
using namespace std;


const float SPEED_NORMAL = 350;//两种模式
const float SPEED_QUICK = 50;
float delay = SPEED_NORMAL;//模式

long addnumproject = 0;
int MAX_SCORE; //最高纪录
FILE* fp;//最高纪录文件
#define Row 21   //21行，最上面的两行的方格并未显示，所以看上去19行
#define Col 11

int table[Row][Col] = { 0 };

bool update = false;

int projectblockIndex;//投影方块的种类

wchar_t BeginningShowRecord[6];//开始的record显示
//鼠标变量
MOUSEMSG m;
MOUSEMSG n;

IMAGE imaBg;
IMAGE imaBgbegin;
IMAGE imaBgend;
IMAGE imaBgAbout;

int blockIndex;//当前方块的种类（1..7）

int nextblockIndex;//下一个方块的种类

// 用来计算时间（每一次下落）
clock_t start, finish;//用来计算单局游戏时长
int Total_time;//单局游戏时长
float timer = 0;




//表示每个方块在第几行第几列
typedef struct
{
	int x;//行数
	int y;//列数
}Point;

Point curBlock[4];  //正在落下的四个方块的各自坐标
Point curnextBlock[4];//下一个四个方块的坐标
Point BakBlock[4];//添加一个备用方块，用来在下降过程中，暂存上一个位置的数据
Point ProjectionBlock[4];//添加一个投影方块，用来表示下落的方块预落地的位置


IMAGE blocksImg[7]; //七种颜色不同的方块图片
IMAGE blocksnextImg[7]; //next方块 的 七种颜色不同的方块图片
IMAGE ProjectionImg[7];//七种颜色不同的对应方块的投影图片
IMAGE numberImg[10];//十个数字对应十个图片

//游戏实时时间的变量
wchar_t timenow_Minute[6];
wchar_t timenow_Second[6];
wchar_t timenow_Hour[6];
int minute = 0;
int hour = 0;
int second = 0;
int time_x = 415;
int time_y = 372;
int Add_x = 2; //移位参数

//游戏结束背景变量组
int score;//分数
int x0_record = 310;
int y0_record = 24;
int x0_score = 317;
int y0_score = 134;
int x0_time = 317;
int y0_time = 253;

//方块种类
int blocks[7][4] = {
	1,3,5,7,//I型
	2,4,5,7,//右弯型
	3,5,4,6,//左弯型
	3,5,4,7,//山型
	2,3,5,7,//倒L型
	3,5,7,6,//正L型
	2,3,4,5 //正方形
};


//打开音乐
void playMainmusic()
{
	//打开音乐
	mciSendString(_T("open MainBGM.mp3 alias MainBGM"), NULL, NULL, NULL);
	//播放音乐
	mciSendString(_T("play MainBGM repeat"), NULL, NULL, NULL);
}

//播放To The Moon
void playAboutmusic()
{
	//关闭背景音乐
	mciSendString(_T("close MainBGM"), NULL, NULL, NULL);

	//打开About音乐
	mciSendString(_T("open To_The_Moon.mp3 alias To_The_Moon"), NULL, NULL, NULL);

	//播放"To The Moon"音乐
	mciSendString(_T("play To_The_Moon repeat"), NULL, NULL, NULL);
}


//About界面
void enter_ABOUT()
{
	playAboutmusic();
	MouseHit();
	//直到摁下退出键
	while (true)
	{
		putimage(0, 0, &imaBgAbout);
		if (MouseHit())
		{//直到有鼠标点击正确位置结束
			n = GetMouseMsg();
			if (n.uMsg == WM_LBUTTONDOWN)
			{
				if (n.x < 104 && n.x>0 && n.y < 67 && n.y>0)break;
			}
		}
	}
	//关闭About音乐
	mciSendString(_T("close To_The_Moon"), NULL, NULL, NULL);
	//打开背景音乐
	playMainmusic();
	n.x = n.y = 0;
}



//读取文件中的数据
void readfile()
{
	//初始化最高纪录文件
	if ((fp = fopen("GameMaxScore.txt", "rb")) == NULL)
	{
		if ((fp = fopen("GameMaxScore.txt", "wb")) == NULL)
		{
			printf("不能创建\" GameMaxScore.txt \"文件\n");
		}
	}
	char ch;
	ch = fgetc(fp);
	if (ch == EOF)
	{
		MAX_SCORE = 0;
	}
	else
	{
		fseek(fp, 0, 0);
		//读取游戏最高纪录
		fread(&MAX_SCORE, sizeof(int), 1, fp);
	}
	fclose(fp);
}
//向文件里写数据
void writefile(int score)
{
	fp = fopen("GameMaxScore.txt", "wb");
	if (score > MAX_SCORE)
	{
		MAX_SCORE = score;
	}
	fwrite(&MAX_SCORE, sizeof(int), 1, fp);
	fclose(fp);
}

void init()
{    //初始化

//创建游戏窗口
	initgraph(580, 745);

	//预加载游戏背景（做准备工作，加载资源）
	//从硬盘中把背景图片文件，“加载”到内存。
	loadimage(&imaBg, _T("Tetris_background_image.bmp"), 580, 745, true);

	loadimage(&imaBgbegin, _T("Tetris_begin_background_image.bmp"), 580, 745, true);

	loadimage(&imaBgend, _T("Tetris_end_background_image.bmp"), 580, 745, true);

	loadimage(&imaBgAbout, _T("Tetris_about_background.png"), 580, 745, true);

	//切割小方块图片
	//1.先把完整的大图片加载到内存
	IMAGE tiles;
	loadimage(&tiles, _T("Tetris_block_picture.bmp"), 31 * 8, 31, true);//true代表等比放大或缩小,现在是一个方块是边长31像素

	IMAGE tilesnext;//下一个方块图需缩小一点
	loadimage(&tilesnext, _T("Tetris_block_picture.bmp"), 31 * 8 / 1.3, 31 / 1.3, true);//true代表等比放大或缩小。

	IMAGE Projection;
	loadimage(&Projection, _T("Tetris_Projection_block.jpg"), 31 * 8, 31, true);//true代表等比放大或缩小。


	//2.对已经加载到内存的大图片，进行切割
	for (int i = 1; i <= 7; i++)
	{
		//设置切割对象：
		SetWorkingImage(&tiles);
		getimage(&blocksImg[i - 1], i * 31, 0, 31, 31);
	}                            //切割的位置， 多宽，多高
	//恢复切割对象为默认对象
	SetWorkingImage();

	//对已经加载到内存的大图片，进行切割（下一个方块）
	for (int i = 1; i <= 7; i++)
	{
		//设置切割对象：
		SetWorkingImage(&tilesnext);
		getimage(&blocksnextImg[i - 1], i * 31 / 1.3, 0, 31 / 1.3, 31 / 1.3);
	}                                   //切割的位置，      多宽，多高
	//恢复切割对象为默认对象
	SetWorkingImage();

	//2.对已经加载到内存的投影大图片，进行切割
	for (int i = 1; i <= 7; i++)
	{
		//设置切割对象：
		SetWorkingImage(&Projection);
		getimage(&ProjectionImg[i - 1], i * 31, 0, 31, 31);
	}                            //切割的位置， 多宽，多高
	//恢复切割对象为默认对象
	SetWorkingImage();

}

//生成一个新方块
void newBlock()
{

	//将下一个方块种类复制到此方块种类
	blockIndex = nextblockIndex;
	projectblockIndex = blockIndex;
	for (int i = 0; i < 4; i++)
	{
		curBlock[i].x = curnextBlock[i].x + 4;
		curBlock[i].y = curnextBlock[i].y;
	}

	//随机选择下一个方块种类
	nextblockIndex = 1 + rand() % 7;

	//初始化下一个俄罗斯方块
	for (int i = 0; i < 4; i++)
	{
		int value = blocks[nextblockIndex - 1][i];
		curnextBlock[i].x = value % 2;
		curnextBlock[i].y = value / 2;
	}

}

//检查操作是否出界
bool check()
{
	for (int i = 0; i < 4; i++)
		if (curBlock[i].x < 0 || curBlock[i].x >= Col ||
			curBlock[i].y >= Row || curBlock[i].y <= 0 ||
			table[curBlock[i].y][curBlock[i].x])
		{
			return false;
		}
	return true;

}

//计算当前正在降落的俄罗斯方块的投影
void Calculateblockshadow()
{
	for (int i = 0; i < 4; i++)
	{
		ProjectionBlock[i] = curBlock[i];
	}

	bool shadow = true;
	while (1) //找到正下方的投影
	{
		for (int i = 0; i < 4; i++)
		{
			if (table[ProjectionBlock[i].y + 1][ProjectionBlock[i].x] != 0 || ProjectionBlock[i].y + 2 > Row)
			{
				shadow = false;
			}
		}

		if (shadow)
		{
			for (int j = 0; j < 4; j++)
			{
				ProjectionBlock[j].y += 1;
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				int x = 23 + ProjectionBlock[i].x * 31 + 1;//1为调整
				int y = 68 + ProjectionBlock[i].y * 31 + 1;
				putimage(x, y, &ProjectionImg[blockIndex - 1]);//将投影方块加入到游戏
			}
			break;
		}
	}
}


void drawblockshadow()
{
	for (int i = 0; i < 4; i++)
	{
		int x = 23 + ProjectionBlock[i].x * 31 + 1;//1为调整
		int y = 68 + ProjectionBlock[i].y * 31 + 1;
		putimage(x, y, &ProjectionImg[blockIndex - 1]);//绘画投影方块
	}
}



void drop()
{  //下降
	for (int i = 0; i < 4; i++)
	{
		BakBlock[i] = curBlock[i];
		curBlock[i].y += 1;
	}

	if (!check())
	{
		for (int i = 0; i < 4; i++)
		{
			// 设置标记，“固化”对应位置
			table[BakBlock[i].y][BakBlock[i].x] = blockIndex;
		}
		newBlock();
		Calculateblockshadow();//计算目前方块的投影方块
		drawblockshadow();//画投影方块
	}
}



void drawBlock()
{
	// 绘制已降落完毕的方块
	for (int i = 0; i < Row; i++)
		for (int j = 0; j < Col; j++)
		{
			if (table[i][j] == 0) continue;
			int x = j * 31 + 23 + 1;
			int y = i * 31 + 68 + 1;
			putimage(x, y, &blocksImg[table[i][j] - 1]);  // 注意一定要减一
		}

	//绘制当前正在降落的俄罗斯方块
	for (int i = 0; i < 4; i++)
	{
		int x = 23 + curBlock[i].x * 31 + 1;//1为调整
		int y = 68 + curBlock[i].y * 31 + 1;
		putimage(x, y, &blocksImg[blockIndex - 1]);    //将方块加入到游戏
	}

	//绘制下一个下落的方块
	if (nextblockIndex == 1)
	{//如果为I型方块，用适当的位置加载。
		for (int i = 0; i < 4; i++)
		{
			int x = 440 + curnextBlock[i].x * 31 / 1.3 + 1;      //1为调整
			int y = 185 + curnextBlock[i].y * 31 / 1.3 + 1;
			putimage(x, y, &blocksnextImg[nextblockIndex - 1]);    //将方块加入到游戏
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			int x = 445 + curnextBlock[i].x * 31 / 1.3 + 1;      //1为调整
			int y = 175 + curnextBlock[i].y * 31 / 1.3 + 1;
			putimage(x, y, &blocksnextImg[nextblockIndex - 1]);    //将方块加入到游戏
		}
	}
}




//定义延时函数，返回距离上次调用有多少毫秒
int getDelay()
{
	static unsigned long long lastTime = 0;
	unsigned long long currentTime = GetTickCount();
	if (lastTime == 0)
	{
		lastTime = currentTime;
		return 0;
	}
	else
	{
		int ret = currentTime - lastTime;
		lastTime = currentTime;
		return ret;
	}
}

//左右移动函数
void moveLeftRight(int offeset)
{
	for (int i = 0; i < 4; i++)
	{
		BakBlock[i] = curBlock[i];  //备份当前方块
		curBlock[i].x += offeset;   //更新当前方块
	}

	if (!check())
	{
		// 把当前方块还原到备份方块
		for (int i = 0; i < 4; i++)
		{
			curBlock[i] = BakBlock[i];
		}
	}
}

//旋转函数
void doRotate(void)
{
	if (blockIndex == 7)
	{  // 田字形，不需要旋转
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		BakBlock[i] = curBlock[i];  //备份当前方块
	}

	Point p = curBlock[1]; //center of rotation
	for (int i = 0; i < 4; i++)
	{
		Point tmp = curBlock[i];
		curBlock[i].x = p.x - tmp.y + p.y;
		curBlock[i].y = p.y + tmp.x - p.x;
	}
	if (!check())
	{
		for (int i = 0; i < 4; i++)
		{
			curBlock[i] = BakBlock[i];
		}
	}
}



//实现削行函数
void clearLine(int& score)
{
	int k = Row - 1;
	for (int i = Row - 1; i > 0; i--)
	{
		int count = 0;
		for (int j = 0; j < Col; j++)
		{
			if (table[i][j]) count++;

			table[k][j] = table[i][j];
		}

		// 如果没有满行,就继续扫描上一行
		// 如果已经满行,k不变,在这个满行内,存放下一次的扫描结果
		if (count < Col) k--;
		else
		{
			score += 10;
			update = true;
		}
	}
}

//检查是否超出上界导致游戏结束
bool checkOver()
{
	for (int j = 0; j < Col; j++)
	{
		if (table[2][j] == 1)return false; //将第2行作为上界，若有1，则游戏结束
	}
	return true;
}


//实现积分
void drawScore(long score)
{
	setcolor(YELLOW);
	settextstyle(60, 0, _T("楷体"));
	wchar_t scoreText[32];
	_itow_s(score, scoreText, 10);//将数字转换为字符串
	setbkmode(TRANSPARENT);
	if (score == 0)
	{
		outtextxy(460, 464, scoreText);
	}
	else if (score >= 10 && score < 100)
	{
		outtextxy(445, 464, scoreText);
	}
	else if (score >= 100 && score < 1000)
	{
		outtextxy(430, 464, scoreText);
	}
}


void Onetouchlanding()
{  //一键降落
	while (check())
	{
		for (int i = 0; i < 4; i++)
		{
			curBlock[i].y += 1;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		curBlock[i].y --;
	}


}


//按键处理函数
/*
要点：方向按键，按下后，会返回两个值
向上： 224   72
向下： 224   80
向左： 224   75
向右： 224   77
空格/enter： 32||13
*/
void keyEvent()
{
	int dx = 0;           //左右移动
	bool rotate = false;  //旋转
	unsigned char ch = 0;
	bool cal = false;//判断是否是有效
	while (_kbhit())
	{
		cal = true;
		unsigned char ch = _getch();
		if (ch == 224)
		{
			ch = _getch();
			printf("%c", ch);
			switch (ch)
			{
			case 72:
				rotate = true;
				break;
			case 80:
				delay = SPEED_QUICK;
				break;
			case 75:
				dx = -1;
				break;
			case 77:
				dx = 1;
				break;
			default:
				cal = false;
			}
		}
		else if (ch == 32 || ch == 13)
		{
			Onetouchlanding(); //一键降落		
			break;
		}
	}

	if (dx != 0)
	{
		moveLeftRight(dx);//左右移动
		update = true;
	}
	if (rotate)
	{
		doRotate();//旋转
		update = true;
	}
	if (cal == true)
	{
		Calculateblockshadow();//重新计算目前方块的投影方块
	}
}

//选择游戏开始或结束
void initmap()
{

	//清空地图
	for (int i = 0; i < Row; i++)
	{
		for (int j = 0; j < Col; j++)
		{
			table[i][j] = 0;
		}
	}
}








void drawEndData(wchar_t* score, wchar_t* MAXrecord, wchar_t* total_time)
{ //画结束时的数据
	settextstyle(80, 0, _T("楷体"));
	setbkmode(TRANSPARENT);
	while (true)
	{
		putimage(0, 0, &imaBgend);
		outtextxy(x0_score, y0_score, score);
		outtextxy(x0_record, y0_record, MAXrecord);
		outtextxy(x0_time, y0_time, total_time);
		while (true)
		{
			if (MouseHit())
			{//直到有鼠标点击正确位置结束
				n = GetMouseMsg();
				if (n.uMsg == WM_LBUTTONDOWN)
					return;
			}
			else
			{
				Sleep(500);
			}
		}
	}
}





void GameTime()
{ //计算游戏实时时间函数
	if (++second == 60)
	{
		second = 0;
		minute++;
		if (minute == 60)
		{
			minute = 0;
			++hour;
		}
	}
	for (int i = 0; i < 2; i++)
	{
		timenow_Hour[i] = timenow_Minute[i] = timenow_Second[i] = '\0';
	}
	_itow_s(hour, timenow_Hour, 10);
	_itow_s(minute, timenow_Minute, 10);
	_itow_s(second, timenow_Second, 10);//将数字转换为字符串
	if (hour != 0)
	{
		time_x -= 20;
		Add_x += 2;
	}
}

void drawNowTime()
{  //显示游戏实时时间函数
	settextcolor(RGB(219, 164, 68));
	settextstyle(40, 0, _T("楷体"));
	if (hour != 0)
	{
		outtextxy(time_x, time_y, timenow_Hour);
		outtextxy(time_x + 20, time_y, _T(":"));
	}
	outtextxy(time_x + 20 * (Add_x - 1), time_y, timenow_Minute);
	outtextxy(time_x + 20 * Add_x + 2, time_y - 5, _T(":"));
	outtextxy((time_x + 20 * (Add_x + 1)), time_y, timenow_Second);
}



int main()
{
	//初始化
	init();
	//游戏开始变量happygame（1为开始，0则退出）
	int happygame = 0;

	MOUSEMSG m;
	putimage(0, 0, &imaBgbegin);
	int a = 0;
	if (a == 0)
	{
		readfile();
		settextcolor(RGB(219, 164, 68));
		settextstyle(20, 0, _T("楷体"));
		_itow_s(MAX_SCORE, BeginningShowRecord, 10);
		setbkmode(TRANSPARENT);
		a++;
	}
	outtextxy(224, 715, _T("Record:"));
	outtextxy(304, 715, BeginningShowRecord);
	//播放游戏背景音乐
	playMainmusic();

	//开始界面
	while (true)
	{
		if (MouseHit())
		{
			m = GetMouseMsg();
			switch (m.uMsg)
			{
			case WM_LBUTTONDOWN:
			{
				if (m.x < 491 && m.x>90 && m.y < 415 && m.y>300)
				{
					happygame = 1;
					m.x = 0; m.y = 0;
					Sleep(500);
					break;
				}
				else if (m.x < 221 && m.x>76 && m.y < 665 && m.y>606)
				{
					happygame = 2;
					m.x = m.y = 0;
					break;
				}
				else if (m.x > 323 && m.y > 610 && m.x < 539 && m.y < 674)
				{
					enter_ABOUT();
					putimage(0, 0, &imaBgbegin);

					settextcolor(RGB(219, 164, 68));
					settextstyle(20, 0, _T("楷体"));
					setbkmode(TRANSPARENT);
					outtextxy(224, 715, _T("Record:"));
					outtextxy(304, 715, BeginningShowRecord);
				}
			}
			}
			if (happygame != 0)break;
		}
	}


	while (happygame == 1)
	{
		//读取游戏纪录文件
		readfile();
		//分数清0
		score = 0;
		//初始化游戏时长
		start = clock();
		Total_time = 0;
		//配置随机种子
		srand(time(NULL));
		nextblockIndex = 1 + rand() % 7;//第一个方块的种类
		//初始化第一个俄罗斯方块
		for (int i = 0; i < 4; i++)
		{
			int value = blocks[nextblockIndex - 1][i];
			curnextBlock[i].x = value % 2;
			curnextBlock[i].y = value / 2;
		}


		putimage(0, 0, &imaBg);
		newBlock(); // 生成第一个方块
		drawBlock();//画方块
		Calculateblockshadow();//计算目前方块的投影方块
		drawblockshadow();//画投影方块



		while (1)
		{
			finish = clock();//结束时间

			drawNowTime();//绘画实时游戏时间
			if ((int)((finish - start) / CLOCKS_PER_SEC) > Total_time)
			{ //修改实时游戏时间
				Total_time = (int)((finish - start) / CLOCKS_PER_SEC);
				GameTime();
			}


			int time = getDelay();
			timer += time;

			keyEvent();

			if (timer > delay)
			{
				drop(); //下降一个位置
				timer = 0;
				update = true;
			}

			clearLine(score); // 清除满行

			delay = SPEED_NORMAL;

			if (update)
			{
				putimage(0, 0, &imaBg);
				drawBlock();       //绘画方块
				drawScore(score);  //绘画分数
				update = false;
			}
			if ((((float)finish - (float)start) / (float)CLOCKS_PER_SEC) - Total_time > 0.7)
			{
				drawblockshadow();//绘画投影方块
				//绘制当前正在降落的俄罗斯方块
				for (int i = 0; i < 4; i++)
				{
					int x = 23 + curBlock[i].x * 31 + 1;//1为调整
					int y = 68 + curBlock[i].y * 31 + 1;
					putimage(x, y, &blocksImg[blockIndex - 1]);    //将方块加入到游戏
				}
			}
			if (checkOver())
			{ //如果第一排没有方块，则继续游戏，否则将游戏结束
				continue;
			}
			else
			{
				break;
			}
		}

		Sleep(1000);
		//存相关数据入文件
		writefile(score);
		happygame = 0;

		//游戏结束页面
		if (happygame == 0)
		{
			setcolor(YELLOW);
			settextstyle(70, 0, _T("楷体"));
			wchar_t Score[10];
			wchar_t Time[10];
			wchar_t Record[10];
			if (score < MAX_SCORE)
			{
				_itow_s(score, Score, 10);//将数字转换为字符串
			}
			else
			{
				_itow_s(MAX_SCORE, Score, 10);
			}
			_itow_s(Total_time, Time, 10);
			_itow_s(MAX_SCORE, Record, 10);
			setbkmode(TRANSPARENT);

			while (true)
			{
				drawEndData(Score, Record, Time);
				if (n.x < 500 && n.x>100 && n.y < 521 && n.y>396)
				{
					happygame = 1;
					n.x = n.y = 0;
					break;
				}
				else if (n.x < 221 && n.x>76 && n.y < 665 && n.y>606)
				{
					happygame = 2;
					n.x = n.y = 0;
					break;
				}
				else if (n.x > 323 && n.y > 610 && n.x < 539 && n.y < 674)
				{
					enter_ABOUT();
					putimage(0, 0, &imaBgbegin);

					settextcolor(RGB(219, 164, 68));
					settextstyle(20, 0, _T("楷体"));
					setbkmode(TRANSPARENT);
					outtextxy(224, 715, _T("Record:"));
					outtextxy(304, 715, BeginningShowRecord);
				}
			}
		}

		if (happygame == 1)initmap();//重新开始新游戏
		if (happygame == 2)break;//退出游戏
	}

	closegraph();
	//closegraph()函数关闭图形模式，释放由图形系统分配的所有内存，并将屏幕恢复为调用initgraph之前的模式。


	return 0;

}

