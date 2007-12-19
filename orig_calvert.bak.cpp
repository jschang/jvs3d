bool jvs3d_terrain::calculateVertices(void)
{	
	int pts[3][3],
		o[3],
		p[3],	// position
		l[3],	// lowest
		r[3],	// greatest 
		i,s;		// temp
	double	slope,	// slope of camera orientation 
			rad,	// radians of slope
			deg;	// degrees of slope
	double	ml,		// the "left" slope
			mr,		// the "right" slope
			me, tmp;		// the capping slope
	double blueColor;
	double yellowColor;
	double magentaColor;
	double testDist, frustEnd;
	jvs3d_vertice 
			rot,
			camLookAt,	// the current position the camera is looking at
			camPos,		// the current position of the camera
			tempPos[4];
	char // for all cai in gai incrementing gai by ai
		gai, // group axis index
		cai, // cell axis index
		ai;  // increment group axis by
	bool 
		stop=false, 
		stop1=false, 
		stop2=false;	// done
	jvs3d_camera *pcam=getCamera();
	memset(o,0,sizeof(int)*3);	// zero out origin point
	memset(p,0,sizeof(int)*3);	// zero out current point;
	memset(r,0,sizeof(int)*3);	// "right" point
	memset(l,0,sizeof(int)*3);	// "left" point

	if(DBGTERR_OL)
	{
	#ifdef _WIN32
		system("cls");
	#else
		system("clear");
	#endif
	}

	if(!pcam) 
		return false;

	if(DBGTERR_OL) 
		printf("Entry test:\n");
	if( pcam->getPosition(camPos) &&
		pcam->getLookAt(camLookAt) && 
		getPosition(&pts[2][0],&pts[2][2],camPos) )
	{
		camLookAt+=camPos;
		if(DBGTERR_OL)
		{
			printf("initial conditions: \n");
			printf("camPos.m_data[0],[1],[2]=%f,%f,%f\n",
				camPos.m_data[0],camPos.m_data[1],camPos.m_data[2] );
			printf("camLookAt.m_data[0],[1],[2]=%f,%f,%f\n",
				camLookAt.m_data[0],camLookAt.m_data[1],camLookAt.m_data[2] );
			printf("pts[2][0],[2]=%u,%u\n\n",pts[2][0],pts[2][2]);
			printf("calculated presets: \n");
		}

		// slope is (from the top x,z) delta z / delta x, right?
		slope = (camPos.m_data[2]-camLookAt.m_data[2]) / 
			(camPos.m_data[0]-camLookAt.m_data[0]);

		m_pCamera->getRotation(rot);
		rad = atan(slope);  // arctan slope to get radians
		deg = fabs(jvs3d_math::RadiansToDegrees(rad));
		if(DBGTERR_OL) 
			printf("slope=%f,rad=%f,deg=%f\n",slope,rad,deg);

		// get the "left" and "right" slope of the fov, 
		// looking down on the map
		ml = tan( rad - (pcam->getFOVRadians()/2) );
		ml = ml>100000 ? (-.087489) : ml;
		mr = tan( rad + (pcam->getFOVRadians()/2) ); 

		frustEnd = pcam->getFrustumEnd() * m_scale;
		if(DBGTERR_OL)
			printf("frustumEnd:%f\n",frustEnd);

		rad = jvs3d_math::DegreesToRadians(270-rot.m_data[1]);  // arctan slope to get radians
		// line which caps the triangl
		pts[0][0] = cos(rad - (pcam->getFOVRadians()/2))*frustEnd+pts[2][0];
		pts[0][2] = sin(rad - (pcam->getFOVRadians()/2))*frustEnd+pts[2][2];
		pts[1][0] = cos(rad + (pcam->getFOVRadians()/2))*frustEnd+pts[2][0];
		pts[1][2] = sin(rad + (pcam->getFOVRadians()/2))*frustEnd+pts[2][2];
		me = (pts[1][0]-pts[0][0]) ?
			(double)(pts[1][2]-pts[0][2])/(double)(pts[1][0]-pts[0][0]) :
			10;

		if(DBGTERR_OL)
			for(i=0;i<3;i++)
				printf("ptr[%d][0],[2]=>%d,%d\n",i,pts[i][0],pts[i][2]);

		if(DBGTERR_VIS)
		{
			getPosition(tempPos[0],pts[0][0],pts[0][2]);
			getPosition(tempPos[1],pts[1][0],pts[1][2]);

			m_pCamera->transform(tempPos[0]);
			m_pCamera->transform(tempPos[1]);

			glColor3f(1,1,0);
			glPointSize(4.0);
			glBegin(GL_POINTS);
				glVertex3dv(tempPos[0].m_data);
				glVertex3dv(tempPos[1].m_data);
			glEnd();
			glPointSize(1.0);
		}

		if(DBGTERR_OL)
		{
			printf("capping:%d,%d;%d,%d;%f\n",o[0],o[2],pts[1][0],pts[1][2],me);
			printf("initial slopes : ml=%f; mr=%f; pcam->getFOVRadians=%f\n",ml,mr,pcam->getFOVRadians());
		}

		// determine whether to subtract z vals
		// or increment, getting each scanline of x along the way
		if( deg<45 || deg>135 )
		// one ascends, the other line descends
		// for each z in x
		{
			gai=0;	// group axis index
			cai=2;	// cell axis index
		} else
		// both are either ascending or descending
		// for each x in z
		{
			gai=2;	// group axis index
			cai=0;	// cell axis index

			// x=f(y), rather than y=f(x)...so invert
			mr = 1/mr;
			ml = 1/ml;
			me = 1/me;
		}
		if(camPos.m_data[gai]<camLookAt.m_data[gai])
			ai=1;	// left slope rises, inc else dec
		else ai=(-1);

		if(DBGTERR_OL) 
		{
			printf("axis adjusted slopes : ml=%f; mr=%f; me=%f\n",ml,mr,me);
			printf("gai=%u,cai=%u,ai=%i\n\nrendered values:\n",gai,cai,ai);
		}

		// formula for line passing through is z = m*x + b
		memcpy(p,o,sizeof(int)*3);
		magentaColor=1.0f;
		yellowColor=1.0f;
		p[0]=pts[2][0];
		p[2]=pts[2][2];
		while( !stop )
		{
			blueColor=1.0f;

			if(p[gai]==0) 
			{
				if(DBGTERR_ML)
					printf("exit@p[gai]==0\n");
				break;
			}

			// get the next to points along the outer edges
			// of our fov
			r[cai]=(p[gai]-pts[2][gai])*mr+pts[2][cai];
			l[cai]=(p[gai]-pts[2][gai])*ml+pts[2][cai];

			// set r to the greatest and l to the lowest
			i=(l[cai]>r[cai]?l[cai]:r[cai]);		// i is the greatest
			l[cai]=(l[cai]<r[cai]?l[cai]:r[cai]);	// l is the lowest
			//i=(i>l[cai]+frustEnd)?l[cai]+frustEnd:i;
			r[cai]=i;

			// less than zero?  then zero
			l[cai]=l[cai]<0?0:l[cai];
			r[cai]=r[cai]<0?0:r[cai];
			r[gai]=p[gai];
			l[gai]=p[gai];

			if( !r[cai] && r[cai]==l[cai] )
			{
				stop=true;
				break;
			}

			o[cai]=(p[gai]-pts[1][gai])*me+pts[1][cai];
			o[gai]=p[gai];
			
			if(DBGTERR_ML) 
				printf("p[%i]=>%i,l[%i]=%i,r[%i]=%i;o[0,2]=%i,%i;\n",
					gai, p[gai],
					cai, l[cai],
					cai, r[cai],
					o[0], o[2]);

			// each vert|horz line per slope diffs
			if(DBGTERR_ML)
				printf("scanline:\n");

			if(DBGTERR_VIS)
			{
				getPosition(tempPos[0],o[0],o[2]);
				m_pCamera->transform(tempPos[0]);
				glColor3f(1,0,1);
				glPointSize(4.0);
				glBegin(GL_POINTS);
					glVertex3dv(tempPos[0].m_data);
				glEnd();
			}

			/*if(ai==1)
				p[cai]=(l[cai]>o[cai]) ? l[cai] : o[cai];
			else p[cai]=(l[cai]<o[cai]) ? l[cai] : o[cai];
			if(ai==-1)
				r[cai]=o[cai]<r[cai]?o[cai]:r[cai];
			else r[cai]=o[cai]>r[cai]?o[cai]:r[cai];*/
			
			/*if( abs(p[gai]-pts[2][gai]) < abs(pts[0][gai]-pts[2][gai]) &&
				abs(p[gai]-pts[2][gai]) < abs(pts[1][gai]-pts[2][gai]) )
				s = ( abs(l[cai]-r[cai]) < abs(o[cai]-r[cai]) ) ? l[cai] : o[cai]+1;
			else */ s = l[cai];
			for( p[cai] = s ; p[cai] < r[cai] ; p[cai]++ )
			{
				if( p[gai]==o[gai] &&
					p[cai]==o[cai] ) break;

				if(p[0]>0)
				{
					// get the position on the height map
					getPosition(tempPos[0],p[0],p[2]);
					getPosition(tempPos[1],p[0]-1,p[2]);
					getPosition(tempPos[2],p[0],p[2]-1);
					getPosition(tempPos[3],p[0]-1,p[2]-1);

					if(DBGTERR_IL)
						printf("(%d,%d,%d),",
							(int)tempPos[0].m_data[0]
							,(int)tempPos[0].m_data[1]
							,(int)tempPos[0].m_data[2]
						);

					// draw the square associated with the position
					if(m_boolDrawGL)
					{
						glBegin(GL_POLYGON);
						if( DBGTERR_VIS )
						{
							if(p[cai]==s)
							{
								glColor3f(yellowColor,yellowColor,0);	// yellow
								yellowColor-=.01f;
							}
							else if(p[cai]==(r[cai]-1))
							{
								glColor3f(0,magentaColor,magentaColor);	// magenta
								magentaColor-=.01f;
							}
							else
							{
								glColor3f(0,0,blueColor);	// blue
								blueColor-=.01f;
							}
						}
						
						for(i=0;i<4;i++)
						{
							m_pCamera->transform(tempPos[i]);
							tempPos[i].drawGL(null);
						}
						glEnd();
					}else
					{
					}
				}
			}
			if(DBGTERR_IL)
				printf("\n");

			// dist from origin is greater than we care about
			testDist = jvs3d_math::getDistance(pts[2][gai],pts[2][cai],r[gai],r[cai]);
			if(DBGTERR_ML) 
				printf("dist=>%f, m_scale=>%f, frustend=>%f\n",
					testDist,m_scale,frustEnd);

			if( testDist > frustEnd ) 
				stop1=true;

			// distance from origin along the other slope
			testDist = jvs3d_math::getDistance(pts[2][gai],pts[2][cai],l[gai],l[cai]);
			if(DBGTERR_ML) 
				printf("dist=>%f, m_scale=>%f, frustend=>%f\n",
					testDist,m_scale,frustEnd);
			if( testDist > frustEnd )
				stop2=true;

			if(stop1&&stop2)
				stop=true;

			if(DBGTERR_ML) printf("\n");

			// inc group axis index by axis increment
			//   plus 1 if slopes are positive
			//   minus 1 if slopes are negative
			p[gai]+=ai;
		}

	} else return false;
	return true;
}
