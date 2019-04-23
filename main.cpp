#include <thread>
#include <iostream>
#include <memory>
#include <ncurses.h>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <mutex>
#include <condition_variable>
#include <algorithm>
using namespace std; 
bool play = true;
int maxX, maxY;
mutex mutexPierwszy;
condition_variable cv;
bool hitWall=false;
int licznik=0;
mutex check;

class Ball
{

	int posX, posY;
	int speedX, speedY;
	int speed;
	bool poruszanie=true;


	public:
	Ball(int posX, int posY,int number, int speed);
	void Moving();
	int getX() const ;
	int getY() const ;
	void setSpeedX(int newSpeed);
	void setSpeedY(int newSpeed);
	void setposX(int newPosX);
	void setposY(int newPosY);
	void setDirection(int number);
	void setPoruszanie(bool wartosc);

	
};


Ball::Ball(int posX, int posY,int number, int speed):posX(posX),posY(posY), speed(speed)
{
	setDirection(number);
}

void Ball::setDirection(int number){

	switch(number)
	{ 
		case 1:{
				speedX=1;
				speedY=1;
			   };break;
		case 2:{
				speedX=1;
				speedY=-1;
			   };break;
		case 3:{
				speedX=-1;
				speedY=-1;

			   };break;
		case 4:{
				speedX=-1;
				speedY=1;

			   };break;
		case 5:{
				speedX=-1;
				speedY=0;

			   };break;	



	}

}

void Ball::setPoruszanie(bool wartosc)
{
	poruszanie = wartosc;
};
void Ball::Moving()
{	

	while(play && poruszanie)
	{	



		if(posX<maxX && posY<maxY && posX>-1  && posY>-1)
		{
			posX=posX+speedX;
			posY=posY+speedY;
			hitWall=false;
		}
		if(posX>=maxX || posX<=0)
		{
			if(posX<=0)speed=150;
			speedX*=-1;
			posX= posX>=maxX ? maxX-1 : 0;
			
			
			hitWall=true;

			cv.notify_all();	

		}	
		if(posY>=maxY || posY<=0)
		{

			speedY*=-1;
			posY= posY>=maxY ? maxY-1 : 0;

			
		
			hitWall=true;

			cv.notify_all();	

		}
		
		
		
		if(speedX>0  )	
		{
			speed= speed-5;
		
		}
		if(speedX<0 )	
		{
			speed=speed+5;
		
		}
		
		
		std::this_thread::sleep_for(std::chrono::milliseconds(speed));
	}
}

int Ball::getX() const 
{
	return posX;
}

int Ball::getY() const
{
	return posY;
}

void Ball::setSpeedX(int newSpeed)
{
	speedX = newSpeed;
}

void Ball::setSpeedY(int newSpeed)
{
	speedY = newSpeed;
}

void Ball::setposX(int newPosX)
{
	posX = newPosX;
}

void Ball::setposY(int newPosY)
{
	posY = newPosY;
}
void nearby(vector<shared_ptr<Ball>> &balls, vector<shared_ptr<thread>> &threads, int i,vector<shared_ptr<thread>> &hitThreads)
{

    bool threadActive = true;
	while(play && threadActive)
	{

		for(int j=0; j<balls.size();j++) 
		{
			if ((balls[i]->getX() - balls[j]->getX())*(balls[i]->getX() - balls[j]->getX()) <= 4 
				&& (balls[i]->getX() - balls[j]->getX())*(balls[i]->getX() - balls[j]->getX()) >= 0 
				&& (balls[i]->getY() - balls[j]->getY())*(balls[i]->getY() - balls[j]->getY()) <= 4 
				&& (balls[i]->getY() - balls[j]->getY())*(balls[i]->getY() - balls[j]->getY()) >= 0  && i!=j) 
			{

				if( (balls[i]->getX() - balls[j]->getX())*(balls[i]->getX() - balls[j]->getX()) <= 4 
				&& (balls[i]->getX() - balls[j]->getX())*(balls[i]->getX() - balls[j]->getX()) > 0 )
				{
					balls[i]->setSpeedX( ((balls[i]->getX() - balls[j]->getX()) >0 ) ? -1 : 1);
					balls[j]->setSpeedX( ((balls[j]->getX() - balls[i]->getX()) >0) ? -1 : 1);

				}

				if((balls[i]->getY() - balls[j]->getY())*(balls[i]->getY() - balls[j]->getY()) <= 4 
				&& (balls[i]->getY() - balls[j]->getY())*(balls[i]->getY() - balls[j]->getY()) > 0)
				{
					balls[i]->setSpeedY(((balls[i]->getY() - balls[j]->getY()) >0 ) ? -1 : 1);
					balls[j]->setSpeedY(((balls[j]->getY() - balls[i]->getY()) >0 ) ? -1 : 1);
				}

				if((balls[i]->getY() - balls[j]->getY())==0)
				{
					balls[i]->setSpeedY(0);
					balls[j]->setSpeedY(0);
				}

				if((balls[i]->getX() - balls[j]->getX())==0)
				{
					balls[i]->setSpeedX(0);
					balls[j]->setSpeedX(0);
				}


				if ((balls[i]->getX() - balls[j]->getX())==0 && (balls[i]->getY() - balls[j]->getY())==0  )
				{
				

				
				balls[j]->setPoruszanie(false);
				balls[j]->setposX(rand()%10000+300);
				balls[j]->setposY(rand()%10000+300);
				unique_lock<mutex> lk(mutexPierwszy);
				
				cv.wait(lk,[]{return hitWall;});
				
				auto newBall = make_shared<Ball>(balls[i]->getX(),balls[i]->getY(),rand()%5+1, 100);
			
				balls[i]->setPoruszanie(false);
				balls[i]->setposX(rand()%10000+300);
				balls[i]->setposY(rand()%10000+300);
				balls.push_back(newBall);
				threads.push_back(move(make_shared<thread>(&Ball::Moving,newBall)));
				hitThreads.push_back(move(make_shared<thread>(nearby, ref(balls),ref(threads),licznik,ref(hitThreads))));
				licznik++;
				lk.unlock();
			
				threadActive = false;
				return;
				}


				
				

			}
			
		}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


void Draw(vector<shared_ptr<Ball>> &balls)
{ 		
	while(play)
	{
		clear();

		for (int i = 0; i < balls.size(); ++i)
		{
			mvprintw(balls[i]->getX(), balls[i]->getY(), "o");
		}
		refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
	}
}


void getExit()
{	
	int sign;

	while(play)
	{
		sign=getch();
		if(sign=='x')
		{
		play=false;
		}

	}

}

int main()
{	
	 
	vector<shared_ptr<Ball>> balls;

	vector<shared_ptr<thread>> threads;
	vector<shared_ptr<thread>> hitThreads;
	
	initscr();
	curs_set(0);
	
	
	getmaxyx(stdscr, maxX, maxY);
	thread makeExit(getExit);
	thread drawing(Draw, ref(balls));
	
	srand(time(NULL));	
	
	while(play)
	{
	
		
		auto newBall = make_shared<Ball>(maxX/2,maxY/2,rand()%5+1, 100);
		balls.push_back(newBall);
		threads.push_back(move(make_shared<thread>(&Ball::Moving,newBall)));
		hitThreads.push_back(move(make_shared<thread>(nearby, ref(balls),ref(threads),licznik,ref(hitThreads))));
		usleep(2000000);
		licznik++;
	}


	for (int i = 0; i < threads.size(); ++i)
	{
		threads[i]->join();
	}

    for (int i = 0; i < hitThreads.size(); ++i)
	{
		hitThreads[i]->join();
		cout<<"ilosc " <<i<<endl;
	}

	makeExit.join();
	drawing.join();

	endwin();
	return 0;
}

