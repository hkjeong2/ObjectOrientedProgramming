#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[7][2] = { { 0, -2.0f }, { -2.0f, -2.0f }, { +2.0f, -2.0f }, { -2.0f, 2.0f }, { +2.0f, 2.0f }, { 0, 2.0f }, { 0, 1.0f } };
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[7] = { d3d::BLUE, d3d::BLUE, d3d::BLUE, d3d::WHITE, d3d::WHITE, d3d::WHITE, d3d::YELLOW };
//hb-re
char score_1 = '0';
char score_2 = '0';

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

int atkDep = 0; // 1 :: team1 attack, 2:: team2 attack, 0 :: start
bool scored = false;

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
	float center_x, center_y, center_z;
	float                   m_radius;
	float               m_velocity_x;
	float               m_velocity_z;
	float defenceArea_x;
	float defenceArea_z;
	float attackArea_x;
	float attackArea_z;

public:
	int teamNum;
	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		m_pSphereMesh = NULL;
	}
	~CSphere(void) {}

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false;
		return true;
	}

	void destroy(void)
	{
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release();
			m_pSphereMesh = NULL;
		}
	}
	void setdefenceArea() {
		defenceArea_x = this->center_x;
		defenceArea_z = this->center_z;
	}
	void setAttackArea() {
		attackArea_x = this->center_x;
		attackArea_z = -this->center_z;
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
	}

	int hasIntersected(CSphere& ball)
	{
		float distance, sumRadius;
		distance = sqrt(pow(this->center_x - ball.center_x, 2) + pow(this->center_z - ball.center_z, 2)); // 두 공 사이의 거리
		sumRadius = ball.getRadius() * 2.0; // 두 구의 반지름 * 2
		if (distance <= sumRadius * 0.95) return 2;
		else if (distance <= sumRadius) return 1;
		else return 0;
	}

	void hitBy(CSphere& ball)
	{
		float disX, disZ, absX, absZ;
		disX = ball.center_x - this->center_x;
		disZ = ball.center_z - this->center_z;
		absX = abs(disX);
		absZ = abs(disZ);

		float degreeA, degreeB, degreeGap;
		degreeA = atan2(disX, disZ);                                //두 공이 충돌시 생기는 각도
		degreeB = atan2(this->m_velocity_x, this->m_velocity_z);    //속도벡터의 각도
		degreeGap = degreeA - degreeB;                              //두 각도의 차이

		double vx, vz;

		if (this->hasIntersected(ball) == 1) {
			vx = -2 * sin(degreeA + degreeGap);
			vz = -2 * cos(degreeA + degreeGap);
			this->setPower(vx, vz);
		}
		else if (this->hasIntersected(ball) == 2) {
			vx = -2 * disX / (absX + absZ);
			vz = -2 * disZ / (absX + absZ);
			this->setPower(vx, vz);
		}
	}
	bool indefenceArea(CSphere& ball) {
		if (ball.center_x < (defenceArea_x + 2.0f) && ball.center_x >(defenceArea_x - 2.0f) && ball.center_z < (defenceArea_z + 2.0f) && ball.center_z >(defenceArea_z - 2.0f))
			return true;
		else
			return false;
	}
	bool inAttackArea(CSphere& ball) {
		if (ball.center_x < (attackArea_x + 2.0f) && ball.center_x >(attackArea_x - 2.0f))
			return true;
		else
			return false;
	}
	void defence(CSphere& ball) {
		if (indefenceArea(ball)) {
			float disX, disZ;
			disX = ball.center_x - this->center_x;
			disZ = ball.center_z - this->center_z;
			float cenDegree;
			cenDegree = atan2(disX, disZ);

			double vx, vz;
			vx = 0.5 * sin(cenDegree);
			vz = 0.5 * cos(cenDegree);

			this->setPower(vx, vz);
		}
		else {
			float disX1, disZ1;
			disX1 = defenceArea_x - this->center_x;
			disZ1 = defenceArea_z - this->center_z;

			float cenDegree;
			cenDegree = atan2(disX1, disZ1);

			double vx1, vz1;
			vx1 = 0.5 * sin(cenDegree);
			vz1 = 0.5 * cos(cenDegree);

			this->setPower(vx1, vz1);
		}
	}
	void defence2(CSphere& ball) {
		if (indefenceArea(ball)) {
			float disX, disZ1, disZ2;
			disX = ball.center_x - this->center_x;
			disZ1 = (ball.center_z - M_RADIUS * 2) - this->center_z;
			disZ2 = (ball.center_z + M_RADIUS * 2) - this->center_z;
			float cenDegree1, cenDegree2;
			cenDegree1 = atan2(disX, disZ1);
			cenDegree2 = atan2(disX, disZ2);

			double vx, vz;
			if (this->teamNum == 1) {
				vx = 0.5 * sin(cenDegree1);
				vz = 0.5 * cos(cenDegree1);
			}
			else {
				vx = 0.5 * sin(cenDegree2);
				vz = 0.5 * cos(cenDegree2);
			}

			this->setPower(vx, vz);
		}
		else {
			float disX1, disZ1;
			disX1 = defenceArea_x - this->center_x;
			disZ1 = defenceArea_z - this->center_z;

			float cenDegree;
			cenDegree = atan2(disX1, disZ1);

			double vx1, vz1;
			vx1 = 0.5 * sin(cenDegree);
			vz1 = 0.5 * cos(cenDegree);

			this->setPower(vx1, vz1);
		}
	}

	void attack(CSphere& ball) {
		if (inAttackArea(ball)) {
			float disX, disZ;
			disX = ball.center_x - this->center_x;
			disZ = ball.center_z - this->center_z;
			float cenDegree;
			cenDegree = atan2(disX, disZ);

			double vx, vz;
			vx = 0.5 * sin(cenDegree);
			vz = 0.5 * cos(cenDegree);

			this->setPower(vx, vz);
		}
		else {
			float disX1, disZ1;
			disX1 = attackArea_x - this->center_x;
			disZ1 = attackArea_z - this->center_z;

			float cenDegree;
			cenDegree = atan2(disX1, disZ1);

			double vx1, vz1;
			vx1 = 0.5 * sin(cenDegree);
			vz1 = 0.5 * cos(cenDegree);

			this->setPower(vx1, vz1);
		}
	}
	void attack2(CSphere& ball) {
		if (inAttackArea(ball)) {
			float vecX, vecZ1, vecZ2;
			vecX = -ball.center_x;
			vecZ1 = 4.0f - ball.center_z;
			vecZ2 = -4.0f - ball.center_z;

			float vecDegree1, vecDegree2;
			vecDegree1 = atan2(vecX, vecZ1);
			vecDegree2 = atan2(vecX, vecZ2);

			float disX, disZ1, disZ2;
			disX = (ball.center_x - (M_RADIUS - 0.02) * 2 * sin(vecDegree1)) - this->center_x;
			disZ1 = (ball.center_z - (M_RADIUS - 0.02) * 2 * cos(vecDegree1)) - this->center_z;
			disZ2 = (ball.center_z - (M_RADIUS - 0.02) * 2 * cos(vecDegree2)) - this->center_z;

			float cenDegree1, cenDegree2;
			cenDegree1 = atan2(disX, disZ1);
			cenDegree2 = atan2(disX, disZ2);

			double vx, vz;
			if (this->teamNum == 1) {
				vx = 0.5 * sin(cenDegree1);
				vz = 0.5 * cos(cenDegree1);
			}
			else {
				vx = 0.5 * sin(cenDegree2);
				vz = 0.5 * cos(cenDegree2);
			}

			this->setPower(vx, vz);
		}
		else {
			float disX1, disZ1;
			disX1 = attackArea_x - this->center_x;
			disZ1 = attackArea_z - this->center_z;

			float cenDegree;
			cenDegree = atan2(disX1, disZ1);

			double vx1, vz1;
			vx1 = 0.5 * sin(cenDegree);
			vz1 = 0.5 * cos(cenDegree);

			this->setPower(vx1, vz1);
		}
	}

	float ballDistance(CSphere& ball) {
		float distance;
		distance = sqrt(pow(this->center_x - ball.center_x, 2) + pow(this->center_z - ball.center_z, 2));
		return distance;
	}

	void setTeamNum(int i) {
		this->teamNum = i;
	}
	int getTeamNum() {
		return this->teamNum;
	}

	void ballUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if (vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
			tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
			tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
			tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
			tZ = 3 - M_RADIUS;*/

			this->setCenter(tX, cord.y, tZ);
		}
		else { this->setPower(0, 0); }
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 0.995;
		if (rate < 0)
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
	}

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;   center_y = y;   center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}


	float getRadius(void)  const { return (float)(M_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;

};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float               m_x;
	float               m_z;
	float               m_width;
	float               m_depth;
	float               m_height;

public:
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_width = iwidth;
		m_depth = idepth;

		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{
		float radius;
		radius = ball.getRadius();
		D3DXVECTOR3 cord = ball.getCenter();
		if (cord.x - radius <= -3.0f || cord.x + radius >= 3.0f || cord.z + radius >= 4.0f || cord.z - radius <= -4.0f)
			return true;
		else return false;
	}
	void hitBy(CSphere& ball)
	{
		float radius;
		radius = ball.getRadius();
		D3DXVECTOR3 cord = ball.getCenter();

		double vx = abs(ball.getVelocity_X());
		double vz = abs(ball.getVelocity_Z());

		double ex_vx = ball.getVelocity_X();
		double ex_vz = ball.getVelocity_Z();


		bool intersect = false;
		intersect = this->hasIntersected(ball);


		if (intersect == true) {
			if (cord.x + radius >= 3.0f) {
				if (ex_vz > 0) ball.setPower(-vx, vz);
				else ball.setPower(-vx, -vz);
			}
			else if (cord.x - radius <= -3.0f) {
				if (ex_vz > 0) ball.setPower(vx, vz);
				else ball.setPower(vx, -vz);
			}
			else if (cord.z + radius >= 4.0 && cord.x >= 1.0f) {
				if (ex_vx > 0) ball.setPower(vx, -vz);
				else ball.setPower(-vx, -vz);
			}
			else if (cord.z + radius >= 4.0 && cord.x <= -1.0f) {
				if (ex_vx > 0) ball.setPower(vx, -vz);
				else ball.setPower(-vx, -vz);
			}
			else if (cord.z - radius <= -4.0 && cord.x <= -1.0f) {
				if (ex_vx > 0) ball.setPower(vx, vz);
				else ball.setPower(-vx, vz);
			}
			else if (cord.z - radius <= -4.0 && cord.x >= 1.0f) {
				if (ex_vx > 0) ball.setPower(vx, vz);
				else ball.setPower(-vx, vz);
			}
			//hb-re
			else if (cord.z > 4.0) {
				ball.setCenter(0.0f, M_RADIUS, 1.0f);
				ball.setPower(0, 0);
				atkDep = 2;
				score_1++;
				scored = true;
			}
			else if (cord.z < -4.0) {
				ball.setCenter(0.0f, M_RADIUS, -3.0f);
				ball.setPower(0, 0);
				atkDep = 1;
				score_2++;
				scored = true;
			}
		}
	}

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }



private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall   g_legoPlane, menuPlane;
CWall   g_legowall[6];
CSphere   g_sphere[7];
CSphere   g_target_blueball;
CLight   g_light;
//hb-re
int time_count = 10;
int game_start = 0;
int blink = 0;
int blink1 = 0;
bool hasStarted = false;

double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}
void initializePosition() {

	for (int i = 0; i < 7; i++) {
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
	}

	if (atkDep == 1) g_sphere[6].setCenter(0.0f, (float)M_RADIUS, -1.0f);
	else if (atkDep == 2) g_sphere[6].setCenter(0.0f, (float)M_RADIUS, 1.0f);
}

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 6, 0.03f, 8, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 6, 0.03f, 8, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 2, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(-2.0f, 0.12f, 4.0f);
	if (false == g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 8.0f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(3.0f, 0.12f, 0.0f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 8.0f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(-3.0f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 2, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(2.0f, 0.12f, 4.0f);
	if (false == g_legowall[4].create(Device, -1, -1, 2, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[4].setPosition(-2.0f, 0.12f, -4.0f);
	if (false == g_legowall[5].create(Device, -1, -1, 2, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[5].setPosition(2.0f, 0.12f, -4.0f);


	// create four balls and set the position
	for (i = 0; i < 7; i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
		g_sphere[i].setdefenceArea();
		g_sphere[i].setAttackArea();
	}
	//set Team
	for (i = 0; i < 3; i++) {
		g_sphere[i].teamNum = 1;
	}
	for (i = 3; i < 6; i++) {
		g_sphere[i].teamNum = 2;
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device, d3d::BLUE)) return false;
	//g_target_blueball.setCenter(.0f, (float)M_RADIUS , .0f);                        ////////////////to change
	g_target_blueball.setCenter(-100.0, -100.0f, -400.0 + M_RADIUS);       //기본 위치 setting

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 12.0f, -1.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 15.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);


	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();
}

//hb-re
void time_text() {
	char temp[100];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont;
	D3DXCreateFont(Device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont);

	RECT rc;
	SetRect(&rc, 750, 30, 0, 0);
	sprintf(temp, "Time Left: %d", time_count);
	g_pFont->DrawText(NULL, temp, -1, &rc, DT_NOCLIP, text_color);

	DWORD dwCurTime = GetTickCount();
	static DWORD dwOldTime = GetTickCount();
	static DWORD dwAccumulateTime = 0; // 누적 시간
	static DWORD m_dwElapsedTime = 0; // 누적 시간
	m_dwElapsedTime = dwCurTime - dwOldTime;
	dwOldTime = dwCurTime;

	dwAccumulateTime += m_dwElapsedTime;
	if (dwAccumulateTime >= 1000)
	{
		dwAccumulateTime = 0;
		time_count--;
	}

	g_pFont->Release();
}
//hb-re
void game_over() {
	char temp[100];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont;
	D3DXCreateFont(Device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont);

	RECT rc;
	SetRect(&rc, 750, 30, 0, 0);
	g_pFont->DrawText(NULL, "Time Left: 0", -1, &rc, DT_NOCLIP, text_color);

	g_pFont->Release();

	LPD3DXFONT g_pFont1;
	D3DXCreateFont(Device, 70, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont1);

	RECT rc1;
	SetRect(&rc1, 360, 300, 0, 0);
	g_pFont1->DrawText(NULL, "GAME OVER", -1, &rc1, DT_NOCLIP, text_color);

	g_pFont1->Release();

	if (score_1 > score_2) {
		LPD3DXFONT g_pFont_1win;
		D3DXCreateFont(Device, 70, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont_1win);

		RECT rc1win;
		SetRect(&rc1win, 380, 380, 0, 0);
		g_pFont_1win->DrawText(NULL, "YOU WIN", -1, &rc1win, DT_NOCLIP, text_color);

		g_pFont_1win->Release();
	}
	else if (score_1 < score_2) {
		LPD3DXFONT g_pFont_2win;
		D3DXCreateFont(Device, 70, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont_2win);

		RECT rc2win;
		SetRect(&rc2win, 400, 380, 0, 0);
		g_pFont_2win->DrawText(NULL, "AI WIN", -1, &rc2win, DT_NOCLIP, text_color);

		g_pFont_2win->Release();
	}
	else {
		LPD3DXFONT g_pFont_draw;
		D3DXCreateFont(Device, 70, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont_draw);

		RECT rcdraw;
		SetRect(&rcdraw, 445, 380, 0, 0);
		g_pFont_draw->DrawText(NULL, "DRAW", -1, &rcdraw, DT_NOCLIP, text_color);

		g_pFont_draw->Release();

	}

	for (int i = 0; i < 7; i++) {
		g_sphere[i].setPower(0, 0);
	}

}

void start_menu() {
	char temp[100];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont1;
	D3DXCreateFont(Device, 70, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont1);

	RECT rc1;
	SetRect(&rc1, 320, 80, 0, 0);
	sprintf(temp, "SOCCER GAME");
	g_pFont1->DrawText(NULL, temp, -1, &rc1, DT_NOCLIP, text_color);

	g_pFont1->Release();
}

void press_sbar() {
	char temp[100];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont;
	D3DXCreateFont(Device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont);

	RECT rc;
	SetRect(&rc, 250, 600, 0, 0);
	sprintf(temp, "- Press SPACE BAR to start game -");
	g_pFont->DrawText(NULL, temp, -1, &rc, DT_NOCLIP, text_color);

	g_pFont->Release();


}

void retry() {
	char temp[100];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont;
	D3DXCreateFont(Device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont);

	RECT rc;
	SetRect(&rc, 250, 600, 0, 0);
	sprintf(temp, "- Press SPACE BAR to restart game -");
	g_pFont->DrawText(NULL, temp, -1, &rc, DT_NOCLIP, text_color);

	g_pFont->Release();


}

void score() {
	char temp[20];
	D3DXCOLOR text_color = d3d::BLACK;

	LPD3DXFONT g_pFont;
	D3DXCreateFont(Device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "돋움체", &g_pFont);


	sprintf(temp, "YOU %c : %c COM", score_1, score_2);
	RECT rc;
	SetRect(&rc, 420, 30, 0, 0);
	g_pFont->DrawText(NULL, temp, -1, &rc, DT_NOCLIP, text_color);

	g_pFont->Release();
}

// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;




	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < 7; i++) {
			g_sphere[i].ballUpdate(timeDelta);
			for (j = 0; j < 6; j++) {
				g_legowall[i].hitBy(g_sphere[6]);
				if (scored == true) {
					initializePosition();
					scored = false;
				}
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < 6; i++) {
			g_legowall[i].draw(Device, g_mWorld);
		}
		for (i = 0; i < 7; i++) {
			g_sphere[i].draw(Device, g_mWorld);
		}


		if (game_start == 0) {
			start_menu();

			DWORD dwCurTime = GetTickCount();
			static DWORD dwOldTime = GetTickCount();
			static DWORD dwAccumulateTime = 0; // 누적 시간
			static DWORD m_dwElapsedTime = 0; // 누적 시간
			m_dwElapsedTime = dwCurTime - dwOldTime;
			dwOldTime = dwCurTime;

			dwAccumulateTime += m_dwElapsedTime;
			if (dwAccumulateTime >= 1000)
			{
				dwAccumulateTime = 0;
				blink++;
			}

			if (blink % 2 == 0)
				press_sbar();

		}


		if (game_start == 1)
		{
			// check whether any two balls hit together and update the direction of balls
			for (i = 0; i < 6; i++) {
				g_sphere[6].hitBy(g_sphere[i]);
			}
			// check atkDep
			for (i = 0; i < 6; i++) {
				if (g_sphere[i].hasIntersected(g_sphere[6]) > 0) {
					atkDep = g_sphere[i].teamNum;
				}
			}
			if (atkDep == 0) {
				for (i = 1; i < 6; i++) {
					g_sphere[i].attack(g_sphere[6]);
				}
			}
			else if (atkDep == 1) {
				for (i = 1; i < 3; i++) {
					g_sphere[i].attack(g_sphere[6]);
				}
				for (i = 3; i < 6; i++) {
					g_sphere[i].defence(g_sphere[6]);
				}
			}
			else {
				for (i = 1; i < 3; i++) {
					g_sphere[i].defence(g_sphere[6]);
				}
				for (i = 3; i < 6; i++) {
					g_sphere[i].attack(g_sphere[6]);
				}
			}
			g_sphere[6].hitBy(g_target_blueball);
			if (g_sphere[6].getCenter().z + M_RADIUS < -4.0f) {     //공 밖으로 나갈 경우
				g_sphere[6].setCenter(g_target_blueball.getCenter().x, g_target_blueball.getCenter().y, g_target_blueball.getCenter().z + 2 * M_RADIUS);
				g_sphere[6].setPower(0, 0);                     //흰 공 위치 파란공 앞으로 조정 및 속도 0으로 setting
			}

			score();

			if (time_count == 0) {
				game_over();
				hasStarted = false;
				DWORD dwCurTime = GetTickCount();
				static DWORD dwOldTime = GetTickCount();
				static DWORD dwAccumulateTime = 0; // 누적 시간
				static DWORD m_dwElapsedTime = 0; // 누적 시간
				m_dwElapsedTime = dwCurTime - dwOldTime;
				dwOldTime = dwCurTime;

				dwAccumulateTime += m_dwElapsedTime;
				if (dwAccumulateTime >= 1000)
				{
					dwAccumulateTime = 0;
					blink1++;
				}

				if (blink1 % 2 == 0)
					retry();
			}
			else
				time_text();
		}

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);

		return true;
	}

}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_LEFT:
		{D3DXVECTOR3 targetpos1 = g_sphere[0].getCenter();
		g_sphere[0].setCenter(targetpos1.x - 0.05, targetpos1.y, targetpos1.z);
		break; }
		case VK_RIGHT:
		{      D3DXVECTOR3 targetpos2 = g_sphere[0].getCenter();
		g_sphere[0].setCenter(targetpos2.x + 0.05, targetpos2.y, targetpos2.z);
		break; }
		case VK_UP:
		{      D3DXVECTOR3 targetpos3 = g_sphere[0].getCenter();
		g_sphere[0].setCenter(targetpos3.x, targetpos3.y, targetpos3.z + 0.05);
		break; }
		case VK_DOWN:
		{      D3DXVECTOR3 targetpos4 = g_sphere[0].getCenter();
		g_sphere[0].setCenter(targetpos4.x, targetpos4.y, targetpos4.z - 0.05);
		break; }
		case VK_SPACE:

			if (hasStarted == false) {
				D3DXVECTOR3 pos(0.0f, 12.0f, -1.0f);
				D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
				D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
				D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
				Device->SetTransform(D3DTS_VIEW, &g_mView);

				game_start = 1;
				time_count = 15;
				for (int i = 0; i < 7; i++) {
					if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
					g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
					g_sphere[i].setPower(0, 0);
					g_sphere[i].setdefenceArea();
				}
				score_1 = '0';
				score_2 = '0';
				hasStarted = true;

				break;
			}
		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		if (LOWORD(wParam) & MK_LBUTTON) {

			if (isReset) {
				isReset = false;
			}
			else {
				D3DXVECTOR3 vDist;
				D3DXVECTOR3 vTrans;
				D3DXMATRIX mTrans;
				D3DXMATRIX mX;
				D3DXMATRIX mY;

				switch (move) {
				case WORLD_MOVE:
					dx = (old_x - new_x) * 0.000f;   //카메라 고정
					dy = (old_y - new_y) * 0.000f;   //카메라 고정

					D3DXMatrixRotationY(&mX, dx);
					D3DXMatrixRotationX(&mY, dy);
					g_mWorld = g_mWorld * mX * mY;

					break;
				}
			}

			old_x = new_x;
			old_y = new_y;



		}
		else {
			isReset = true;
			if (LOWORD(wParam) & MK_RBUTTON) {

				dx = (old_x - new_x);
				dy = (old_y - new_y);

				D3DXVECTOR3 coord3d = g_sphere[0].getCenter();

				if (dx > 999 || dx < -999) {        //화면 밖으로 나갈 시 움직인 값 0으로 setting
					dx = 0;
					new_x = 0;
				}

				if (coord3d.x <= -3.0f + M_RADIUS)      //왼쪽으로 마우스 커서 나갈 경우
					g_sphere[0].setCenter(-2.99f + M_RADIUS, coord3d.y, coord3d.z);

				else if (coord3d.x >= 3.0f - M_RADIUS)  //오른쪽으로 나갈 경우
					g_sphere[0].setCenter(2.99f - M_RADIUS, coord3d.y, coord3d.z);

				else    //when moving in map
					g_sphere[0].setCenter(coord3d.x + dx * (-0.009f), coord3d.y, coord3d.z + dy * (0.009f));

			}

			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
		}
		break;
	}
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}