#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzStdStructElements.c
* Date:         March 1999
* Author:       Richard Baldock
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Procedures to generate standard structuring elements
*		in 2D and 3D.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
* 03-03-2K bill	Replace WlzPushFreePtr(), WlzPopFreePtr() and 
*		WlzFreeFreePtr() with AlcFreeStackPush(),
*		AlcFreeStackPop() and AlcFreeStackFree().
************************************************************************/
#include <stdlib.h>
#include <math.h>

#include <Wlz.h>

/*
 * 	010
 *	111
 *	010
 */

static WlzObject *WlzSpecial_h4(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzDomain	domain;
  WlzValues	values;
  WlzInterval *intl;
  int 	i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL, 
				       -1, 1, -1, 1, &errNum) ){
    values.core = NULL;
    if( obj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
			  NULL, NULL, &errNum) ){
      intl = (WlzInterval *) AlcMalloc(sizeof(WlzInterval)*3);
      domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr, (void *)intl,
      					   NULL);
      for(i = -1; i < 2; i++){
	if(i == 0){
	  intl->ileft = 0;
	  intl->iright = 2;
	} else {
	  intl->ileft = 1;
	  intl->iright = 1;
	}
	WlzMakeInterval(i, domain.i, 1, intl);
	intl++;
      }
    }
    else {
      WlzFreeIntervalDomain(domain.i);
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}
/*
 *	111
 *	010
 *	111
 */
static WlzObject *WlzSpecial_ex4(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzDomain	domain;
  WlzValues	values;
  WlzInterval *intl;
  int 	i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL, 
				       -1, 1, -1, 1, &errNum) ){
    values.core = NULL;
    if( obj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
			  NULL, NULL, &errNum) ){
      intl = (WlzInterval *) AlcMalloc(sizeof(WlzInterval)*3);
      domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr, (void *)intl,
      					   NULL);
      for(i = -1; i < 2; i++){
	if(i == 0){
	  intl->ileft = 1;
	  intl->iright = 1;
	} else {
	  intl->ileft = 0;
	  intl->iright = 2;
	}
	WlzMakeInterval(i, domain.i, 1, intl);
	intl++;
      }
    }
    else {
      WlzFreeIntervalDomain(domain.i);
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}
/*
 *	111
 *	111
 *	111
 */	
static WlzObject *WlzSpecial_a8(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzDomain	domain;
  WlzValues	values;
  WlzInterval *intl;
  int 	i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL, 
				       -1, 1, -1, 1, &errNum) ){
    values.core = NULL;
    if( obj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
			  NULL, NULL, &errNum) ){
      intl = (WlzInterval *) AlcMalloc(sizeof(WlzInterval)*3);
      domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr, (void *)intl,
      					   NULL);
      for(i = -1; i < 2; i++){
	intl->ileft = 0;
	intl->iright = 2;
	WlzMakeInterval(i, domain.i, 1, intl);
	intl++;
      }
    }
    else {
      WlzFreeIntervalDomain(domain.i);
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}

/*
 *  0:		 1:     	2:	3:	4:	5:    
 *    011	   110  	  111     110     010     011       
 *    111          111         	  111     111     111     111
 *    110          011            010     110     111     011
 */
static WlzObject *WlzSpecial_h6(int num, WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzInterval	*intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzSpecial_a8(&errNum) ){
    intl = obj->domain.i->intvlines->intvs;
    switch(num){
    case 0:
      intl->iright = 1;
      intl += 2;
      intl->ileft = 1;
      break;
    case 1:
      intl->ileft = 1;
      intl += 2;
      intl->iright = 1;
      break;
    case 2:
      intl->ileft = 1;
      intl->iright = 1;
      break;
    case 3:
      intl->iright = 1;
      intl += 2;
      intl->iright = 1;
      break;
    case 4:
      intl += 2;
      intl->ileft = 1;
      intl->iright = 1;
      break;
    case 5:
      intl->ileft += 1;
      intl += 2;
      intl->ileft += 1;
      break;
    default:
      errNum=WLZ_ERR_PARAM_DATA;
      WlzFreeObj(obj);
      obj = NULL;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}

/*
 *	0:	1:	2:	3:
 *	  010	  010	  011	  110
 * 	  111	  111	  111 	  111
 *	  110	  011	  010	  010
 */
static WlzObject *WlzSpecial_h5(int num, WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzInterval *intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzSpecial_h4(&errNum) ){
    intl = obj->domain.i->intvlines->intvs;
    switch(num){
    case 0:
      intl->ileft = 0;
      break;
    case 1:
      intl->iright = 2;
      break;
    case 2:
      intl += 2;
      intl->iright = 2;
      break;
    case 3:
      intl += 2;
      intl->ileft = 0;
      break;
    default:
      errNum=WLZ_ERR_PARAM_DATA;
      WlzFreeObj(obj);
      obj = NULL;
      break;
    }
  }


  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}

/*
 *	0:	1:	2:	3:
 *	  111	  111	  110	  011
 * 	  111	  111	  111 	  111
 *	  011	  110	  111	  111
 */
static WlzObject *WlzSpecial_h7(int num, WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzInterval	*intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzSpecial_a8(&errNum) ){
    intl = obj->domain.i->intvlines->intvs;
    switch(num){
    case 0:
      intl->ileft = 1;
      break;
    case 1:
      intl->iright = 1;
      break;
    case 2:
      intl += 2;
      intl->iright = 1;
      break;
    case 3:
      intl += 2;
      intl->ileft = 1;
      break;
    default:
      errNum=WLZ_ERR_PARAM_DATA;
      WlzFreeObj(obj);
      obj = NULL;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}
/*
 * num = 0:	1:	2:	3:
 *	   000	  010	  010	  010
 *	   111    011	  111	  110
 *	   010	  010	  000	  010
 */
static WlzObject *WlzSpecial_a3(int num, WlzErrorNum *dstErr)  /*-*/
{
  WlzObject 	*obj=NULL;
  WlzInterval *intl;
  int 	i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzSpecial_a8(&errNum) ){
    switch(num){
    case 0:
      intl = obj->domain.i->intvlines[0].intvs;
      intl->ileft = 1;
      intl->iright = 1;
      obj->domain.i->intvlines[2].nintvs = 0;
      WlzStandardIntervalDomain(obj->domain.i);
      break;
    case 1:
      intl = obj->domain.i->intvlines[0].intvs;
      for(i = -1; i < 2; i++){
	intl->ileft = 1;
	if(i == 0){
	  intl->iright = 2;
	}
	else intl->iright = 1;
	intl++;
      }
      WlzStandardIntervalDomain(obj->domain.i);
      break;
    case 2:
      intl = obj->domain.i->intvlines[2].intvs;
      intl->ileft = 1;
      intl->iright = 1;
      obj->domain.i->intvlines[0].nintvs = 0;
      WlzStandardIntervalDomain(obj->domain.i);
      break;
    case 3:
      intl = obj->domain.i->intvlines[0].intvs;
      for(i = -1; i < 2; i++){
	if(i == 0)
	  intl->ileft = 0;
	else intl->ileft = 1;
	intl->iright = 1;
	intl++;
      }
      WlzStandardIntervalDomain(obj->domain.i);
      break;
    default:
      errNum=WLZ_ERR_PARAM_DATA;
      WlzFreeObj(obj);
      obj = NULL;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}
/*
 * 11
 */
static WlzObject *WlzSpecial_e1(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzInterval	*intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzMakeSinglePixelObject(WLZ_2D_DOMAINOBJ, 0, 0, 0, &errNum) ){
    intl = obj->domain.i->intvlines->intvs;
    intl->ileft = 0;
    intl->iright = 1;
    obj->domain.i->lastkl = 1;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return obj;
}
/*
 * 111
 */
static WlzObject *WlzSpecial_e2(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj=NULL;
  WlzInterval	*intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj = WlzMakeSinglePixelObject(WLZ_2D_DOMAINOBJ, 0, 0, 0, &errNum) ){
    intl = obj->domain.i->intvlines->intvs;
    intl->ileft = 0;
    intl->iright = 2;
    obj->domain.i->kol1 = -1;
    obj->domain.i->lastkl = 1;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return obj;
}

/*
 * 1
 * 1
 * 1
 */
static WlzObject *WlzSpecial_v2(WlzErrorNum *dstErr)  /*-*/
{
  WlzObject	*obj1, *obj2, *obj=NULL;
  WlzInterval	*intl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  if( obj1 = WlzSpecial_a3(1, &errNum) ){
    if( obj2 = WlzSpecial_a3(3, &errNum) ){
      obj = WlzIntersect2(obj1, obj2, &errNum);
      WlzFreeObj(obj2);
    }
    WlzFreeObj(obj1);
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return obj;
}

WlzObject *WlzMakeSpecialStructElement(
  WlzSpecialStructElmType	eType,
  int				elmIndex,
  WlzErrorNum			*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* switch on type calling each of Liang's specials as required */
  switch( eType ){
  case WLZ_SPEC_STRUCT_ELM_H4:
    rtnObj = WlzSpecial_h4(&errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_EX4:
    rtnObj = WlzSpecial_ex4(&errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_A8:
    rtnObj = WlzSpecial_a8(&errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_H6:
    rtnObj = WlzSpecial_h6(elmIndex, &errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_H5:
    rtnObj = WlzSpecial_h5(elmIndex, &errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_H7:
    rtnObj = WlzSpecial_h7(elmIndex, &errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_A3:
    rtnObj = WlzSpecial_a3(elmIndex, &errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_E1:
    rtnObj = WlzSpecial_e1(&errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_E2:
    rtnObj = WlzSpecial_e2(&errNum);
    break;

  case WLZ_SPEC_STRUCT_ELM_V2:
    rtnObj = WlzSpecial_v2(&errNum);
    break;

  default:
    errNum = WLZ_ERR_PARAM_TYPE;
    break;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}


WlzObject *WlzMakeSinglePixelObject(
  WlzObjectType	oType,
  int		k,
  int		l,
  int		p,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzInterval	*itv;
  WlzDomain	domain;
  WlzValues	values;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check type */
  switch( oType ){
  case WLZ_2D_DOMAINOBJ:
    if((itv = (WlzInterval *)AlcMalloc(sizeof(WlzInterval))) == NULL) {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else {
      itv->ileft = k;
      itv->iright = k;
      if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL,
					   l, l, k, k, &errNum) ){
	domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr, itv,
					     NULL);
	domain.i->intvlines->nintvs = 1;
	domain.i->intvlines->intvs = itv;
	values.core = NULL;
	if( (rtnObj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
				  NULL, NULL, &errNum)) == NULL ){
	  WlzFreeIntervalDomain(domain.i);
	}
      }
    }
    break;

  case WLZ_3D_DOMAINOBJ:
    if( domain.p = WlzMakePlaneDomain(WLZ_PLANEDOMAIN_DOMAIN,
				      p, p, l, l, k, k, &errNum) ){
      values.core = NULL;
      if( (rtnObj = WlzMakeMain(WLZ_3D_DOMAINOBJ, domain, values,
				NULL, NULL, &errNum)) == NULL ){
	WlzFreePlaneDomain(domain.p);
	break;
      }
      if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL,
					 l, l, k, k, &errNum) ){
	rtnObj->domain.p->domains[0] = WlzAssignDomain(domain, NULL);
      }
      else {
	WlzFreeObj(rtnObj);
	rtnObj = NULL;
      }
    }
    break;

  default:
    errNum = WLZ_ERR_PARAM_TYPE;
    break;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

WlzObject *WlzMakeCircleObject(
  double	radius,
  double	x,
  double	y,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzDomain	domain;
  WlzValues	values;
  WlzInterval	*intvlPtr;
  int		line1, lastln, kol1, lastkl, width, idelta;
  int		l;
  double	delta;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* only error checked for is a negative radius */
  if( radius < 0.0 ){
    errNum = WLZ_ERR_PARAM_DATA;
  }
  else if( radius < 0.5 ){
    rtnObj = WlzMakeSinglePixelObject(WLZ_2D_DOMAINOBJ,
				      WLZ_NINT(x), WLZ_NINT(y), 0,
				      &errNum);
  }
  else {
    line1 = WLZ_NINT(y-radius);
    lastln = WLZ_NINT(y+radius);
    kol1 = WLZ_NINT(x-radius);
    lastkl = WLZ_NINT(x+radius);
    width = lastkl - kol1 + 1;
    if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL,
					 line1, lastln, kol1, lastkl,
					 &errNum) ){
      if( intvlPtr = (WlzInterval *) AlcCalloc((lastln - line1 + 1),
					       sizeof(WlzInterval)) ){
	domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr,
					     (void *)intvlPtr, NULL);
	for(l=line1; l <= lastln; l++, intvlPtr++){
	  delta = radius*radius - (l-y)*(l-y);
	  if( delta < 0.5 ){
	    idelta = 0;
	  }
	  else {
	    idelta = (int) sqrt(delta);
	  }
	  intvlPtr->ileft = width/2 - idelta;
	  intvlPtr->iright = width/2 + idelta;
	  WlzMakeInterval(l, domain.i, 1, intvlPtr);
	}
	WlzStandardIntervalDomain(domain.i);
	values.core = NULL;
	if( (rtnObj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
				  NULL, NULL, &errNum)) == NULL ){
	  WlzFreeIntervalDomain(domain.i);
	}
      }
      else {
	errNum = WLZ_ERR_MEM_ALLOC;
	WlzFreeIntervalDomain(domain.i);
      }
    }
  }
    

  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

WlzObject *WlzMakeSphereObject(
  WlzObjectType	oType,
  double	radius,
  double	x,
  double	y,
  double	z,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzObject	*obj1;
  WlzDomain	domain, *domains;
  WlzValues	values;
  double	tmpRadius;
  int		p, plane1, lastpl;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check the radius - the only error is if it is negative */
  if( radius < 0.0 ){
    errNum = WLZ_ERR_PARAM_DATA;
  }
  else {
    /* check type - 2D domain implies circular object */
    switch( oType ){
    case WLZ_2D_DOMAINOBJ:
      return WlzMakeCircleObject(radius, x, y, &errNum);

    case WLZ_3D_DOMAINOBJ:
      plane1 = WLZ_NINT(z-radius);
      lastpl = WLZ_NINT(z+radius);
      if( domain.p = WlzMakePlaneDomain(WLZ_PLANEDOMAIN_DOMAIN,
					plane1, lastpl,
					WLZ_NINT(y-radius),
					WLZ_NINT(y+radius),
					WLZ_NINT(x-radius),
					WLZ_NINT(x+radius),
					&errNum) ){
	values.core = NULL;
	if( (rtnObj = WlzMakeMain(WLZ_3D_DOMAINOBJ, domain, values,
				  NULL, NULL, &errNum)) == NULL ){
	  WlzFreePlaneDomain(domain.p);
	  break;
	}
	domains = domain.p->domains;
	for(p=plane1; p <= lastpl; p++, domains++){
	  tmpRadius = radius*radius - (p-z)*(p-z);
	  if( tmpRadius < 1.0 ){
	    tmpRadius = 1.0;
	  }
	  else {
	    tmpRadius = sqrt(tmpRadius);
	  }
	  if( obj1 = WlzMakeCircleObject(tmpRadius, x, y, &errNum) ){
	    *domains = WlzAssignDomain(obj1->domain, NULL);
	    WlzFreeObj(obj1);
	  }
	  else {
	    WlzFreeObj(rtnObj);
	    rtnObj = NULL;
	    break;
	  }
	}
      }
      break;

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  if( rtnObj ){
    WlzStandardPlaneDomain(rtnObj->domain.p, NULL);
  }
  return rtnObj;
}


WlzObject *WlzMakeStdStructElement(
  WlzObjectType		oType,
  WlzDistanceType	dType,
  double		radius,
  WlzErrorNum		*dstErr)
{
  WlzObject	*structElm=NULL;
  WlzObject	*obj1, *obj2;
  int 		intRadius;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check the radius - convert to nearest integer for all except
     Euclidean. Even then r < 0.5 => an empty structuring element.
     -ve radius is an error */
  if( radius < 0.0 ){
    errNum = WLZ_ERR_PARAM_DATA;
  }
  else if( radius < 0.5 ){
    return WlzMakeEmpty(dstErr);
  }
  else {
    intRadius = WLZ_NINT(radius);
  }

  /* now check the distance type */
  if( errNum == WLZ_ERR_NONE ){
    switch( dType ){
    case WLZ_8_DISTANCE:
    case WLZ_4_DISTANCE:
    case WLZ_6_DISTANCE:
    case WLZ_18_DISTANCE:
    case WLZ_26_DISTANCE:
      /* make a domain with a single pixel at the origin and dilate
	 to the required radius */
      if( obj1 = WlzMakeSinglePixelObject(oType, 0, 0, 0, &errNum) ){
	while( intRadius-- > 0 ){
	  if( obj2 = WlzDilation(obj1, (WlzConnectType) dType, &errNum) ){
	    WlzFreeObj(obj1);
	    obj1 = obj2;
	  }
	  else {
	    WlzFreeObj(obj1);
	    obj1 = NULL;
	    break;
	  }
	}
	structElm = obj1;
      }
      break;
	
    case WLZ_OCTAGONAL_DISTANCE:
      /* make a domain with a single pixel at the origin and dilate
	 alternately 8-conn, 4-conn to required radius */
      if( obj1 = WlzMakeSinglePixelObject(oType, 0, 0, 0, &errNum) ){
	while( intRadius-- > 0 ){
	  if( intRadius & 0x01 ){
	    obj2 = WlzDilation(obj1, WLZ_4_CONNECTED, &errNum);
	  }
	  else {
	    obj2 = WlzDilation(obj1, WLZ_8_CONNECTED, &errNum);
	  }

	  if( obj2 ){
	    WlzFreeObj(obj1);
	    obj1 = obj2;
	  }
	  else {
	    WlzFreeObj(obj1);
	    obj1 = NULL;
	    break;
	  }
	}
	structElm = obj1;
      }
      break;
	
    case WLZ_EUCLIDEAN_DISTANCE:
      structElm = WlzMakeSphereObject(oType, radius,
				      0.0, 0.0, 0.0, &errNum);
      break;

    default:
      errNum = WLZ_ERR_PARAM_TYPE;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return structElm;
}
