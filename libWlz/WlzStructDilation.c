#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzStructDilation.c
* Date:         March 1999
* Author:       Richard Baldock
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Performs dilation using a structuring element.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
* 03-03-2K bill	Replace WlzPushFreePtr(), WlzPopFreePtr() and 
*		WlzFreeFreePtr() with AlcFreeStackPush(),
*		AlcFreeStackPop() and AlcFreeStackFree().
************************************************************************/
#include <stdlib.h>
#include <Wlz.h>

#define WLZ_STRUCT_DILATE_MAXLNITV 50

static int unionitvs(WlzIntervalLine	*itva,
		     WlzIntervalLine	*itvb,
		     int		n,
		     WlzInterval	*jp,
		     WlzInterval	*aa,
		     WlzInterval	*bb,
		     WlzInterval	*cc);

static int line_struct_dil(WlzIntervalLine	*itvl,
			   WlzInterval		*jntl,
			   WlzInterval		*ll);

static int arr_arr_union(WlzInterval	*aa,
			 int		m,
			 WlzInterval	*bb,
			 int		n,
			 WlzInterval	*cc);

static WlzObject *WlzStructDilation3d(WlzObject		*obj,
				      WlzObject		*structElm,
				      WlzErrorNum	*dstErr);

/************************************************************************
*   Function   : WlzStructDilation					*
*   Date       : Mon Nov  3 07:36:01 1997				*
*************************************************************************
*   Synopsis   :							*
*   Returns    :							*
*   Parameters :							*
*   Global refs:							*
************************************************************************/

WlzObject *WlzStructDilation(
  WlzObject	*obj,
  WlzObject	*structElm,
  WlzErrorNum	*dstErr)
{
  WlzObject 		*rtnObj, *bObj, *sObj;
  WlzDomain		bDom,
  			sDom,
  			domain;
  WlzValues		values;
  WlzInterval 		*jp, *jpw;
  WlzIntervalLine 	*bitv, *sitv;
  int 			i, j, k;
  int 			n = 0, m, size;
  int 			line1, kol1, lastln, lastkl;
  WlzInterval 		aa[2 * WLZ_STRUCT_DILATE_MAXLNITV];
  WlzInterval 		bb[2 * WLZ_STRUCT_DILATE_MAXLNITV];
  WlzInterval 		cc[2 * WLZ_STRUCT_DILATE_MAXLNITV];
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  bDom.core = NULL;
  sDom.core = NULL;
  values.core = NULL;
  /* check the object */
  if( obj == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else {
    switch( obj->type ){
    case WLZ_2D_DOMAINOBJ:
      /* check the domain */
      if( obj->domain.core == NULL ){
	errNum = WLZ_ERR_DOMAIN_NULL;
      }
      else if( obj->domain.i->type == WLZ_EMPTY_DOMAIN ){
	return WlzMakeEmpty(dstErr);
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      return WlzStructDilation3d(obj, structElm, dstErr);

    case WLZ_TRANS_OBJ:
      if( values.obj = WlzStructDilation(obj->values.obj, structElm,
					&errNum) ){
	return WlzMakeMain(WLZ_TRANS_OBJ, obj->domain, values,
			   NULL, NULL, dstErr);
      }
      break;

    case WLZ_EMPTY_OBJ:
      return WlzMakeEmpty(dstErr);

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  /* check the structuring element */
  if( errNum == WLZ_ERR_NONE ){
    if( structElm == NULL ){
      errNum = WLZ_ERR_OBJECT_NULL;
    }
    else {
      switch( structElm->type ){
      case WLZ_2D_DOMAINOBJ:
	/* check the domain */
	if( structElm->domain.core == NULL ){
	  errNum = WLZ_ERR_DOMAIN_NULL;
	}
	else if( structElm->domain.i->type == WLZ_EMPTY_DOMAIN ){
	  WlzMakeMain(obj->type, obj->domain, values,
		      NULL, NULL, dstErr);
	}
	break;

      case WLZ_TRANS_OBJ:
	return WlzStructDilation(obj, structElm->values.obj, &errNum);

      case WLZ_EMPTY_OBJ:
	return WlzMakeMain(obj->type, obj->domain, values,
			   NULL, NULL, dstErr);

      default:
	errNum = WLZ_ERR_OBJECT_TYPE;
	break;
      }
    }
  }

  if( errNum == WLZ_ERR_NONE ){
    /*
     * use smaller object as the structuring element
     */
    i = obj->domain.i->lastln - obj->domain.i->line1;
    j = structElm->domain.i->lastln - structElm->domain.i->line1;
    if(j < i){
      sObj = structElm;
      bObj = obj;
      k = j;
    } else {
      sObj = obj;
      bObj = structElm;
      k = i;
    }
  }
  /*
   * ensure that the domains are interval interval domains.
   */
  if(errNum == WLZ_ERR_NONE)
  {
    if(bObj->domain.i->type == WLZ_INTERVALDOMAIN_INTVL)
    {
      bDom = WlzAssignDomain(bObj->domain, NULL);
    }
    else
    {
      bDom.i = WlzNewIDomain(WLZ_INTERVALDOMAIN_INTVL, bObj->domain.i,
			     &errNum);
      (void )WlzAssignDomain(bDom, NULL);
    }
  }
  if(errNum == WLZ_ERR_NONE)
  {
    if(sObj->domain.i->type == WLZ_INTERVALDOMAIN_INTVL)
    {
      sDom = WlzAssignDomain(sObj->domain, NULL);
    }
    else
    {
      sDom.i = WlzNewIDomain(WLZ_INTERVALDOMAIN_INTVL, sObj->domain.i,
			     &errNum);
      (void )WlzAssignDomain(sDom, NULL);
    }
  }
  if( errNum == WLZ_ERR_NONE ){
    /*
     * space for the intervals of the return object
     */
    size = WlzIntervalCount(sDom.i, NULL) * WlzIntervalCount(bDom.i, NULL);
    if( jp = (WlzInterval *) AlcMalloc(sizeof(WlzInterval) * size) ){
      jpw = jp;
      /*
       * working space
       */
      k++;
      line1 = bDom.i->line1 + sDom.i->line1;
      lastln = bDom.i->lastln + sDom.i->lastln;
      kol1 = bDom.i->kol1 + sDom.i->kol1;
      lastkl = bDom.i->lastkl + sDom.i->lastkl;
      n = 1;
      m = line1 + k - 1;
      if( domain.i = WlzMakeIntervalDomain(WLZ_INTERVALDOMAIN_INTVL,
					 line1, lastln, kol1, lastkl,
					 &errNum) ){
	domain.i->freeptr = AlcFreeStackPush(domain.i->freeptr, (void *)jpw,
					     NULL);
	rtnObj = WlzMakeMain(WLZ_2D_DOMAINOBJ, domain, values,
			     NULL, NULL, &errNum);
      }
    }
    else {
      errNum = WLZ_ERR_MEM_ALLOC;
    }
  }

  /*
   * scanning from first line to last line
   */
  if( errNum == WLZ_ERR_NONE ){
    bitv = &(bDom.i->intvlines[0]);
    sitv = &(sDom.i->intvlines[0]);
    for(i = line1; i < m; i++){
      j = unionitvs(bitv, sitv, n++, jp, aa, bb, cc);
      WlzMakeInterval(i, domain.i, j, jp);
      jp += j;
      sitv++;
    }
    n = lastln - k + 2;
    for(i = m; i < n; i++){
      j = unionitvs(bitv, sitv, k, jp, aa, bb, cc);
      WlzMakeInterval(i, domain.i, j, jp);
      jp += j;
      bitv++;
    }
    m = k - 1;
    for(i = n; i <= lastln; i++){
      j = unionitvs(bitv, sitv, m--, jp, aa, bb, cc);
      WlzMakeInterval(i, domain.i, j, jp);
      jp += j;
      bitv++;
    }
  }
  /*
   * copy the domain to minimise the allocated space
   */
  if( errNum == WLZ_ERR_NONE ){
    if( domain.i = WlzNewIDomain(WLZ_INTERVALDOMAIN_INTVL,
    				 rtnObj->domain.i, &errNum) ){
      WlzFreeIntervalDomain(rtnObj->domain.i);
      rtnObj->domain = WlzAssignDomain(domain, NULL);
    }
    else {
      WlzFreeObj(rtnObj);
      rtnObj = NULL;
    }
  }
  if(bDom.core)
  {
    (void )WlzFreeDomain(bDom);
  }
  if(sDom.core)
  {
    (void )WlzFreeDomain(sDom);
  }
  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

/************************************************************************
*   Function   : unionitvs						*
*   Date       : Mon Nov  3 07:35:17 1997				*
*************************************************************************
*   Synopsis   :							*
*   Returns    :							*
*   Parameters :							*
*   Global refs:							*
************************************************************************/

static int unionitvs(
  WlzIntervalLine	*itva,
  WlzIntervalLine	*itvb,
  int			n,
  WlzInterval		*jp,
  WlzInterval		*aa,
  WlzInterval		*bb,
  WlzInterval		*cc)
{
  WlzInterval *intl;
  int i, j, k, m;
  int pp;

  if(n == 0)
    return(0);
  /*
   * The result of intervals dilated by an interval put in ll and rr
   * except the first time
   * The result of line's union is kept in lc, rc and lb, rb in turn
   * depending on even line or odd line
   */
  k = 0;
  m = 0;
  pp = 0;
  for(i = 0; i < n; i++){
    if(itva->nintvs < itvb->nintvs){
      intl = itva->intvs;
      for(j = 0; j < itva->nintvs; j++){
	k = line_struct_dil(itvb, intl, aa);
	if(pp == 0){
	  m = arr_arr_union(aa, k, bb, m, cc);
	  pp = 1;
	} else {
	  m = arr_arr_union(aa, k, cc, m, bb);
	  pp = 0;
	}
	intl++;
      }
    } else {
      intl = itvb->intvs;
      for(j = 0; j < itvb->nintvs; j++){
	k = line_struct_dil(itva, intl, aa);
	if(pp == 0){
	  m = arr_arr_union(aa, k, bb, m, cc);
	  pp = 1;
	} else {
	  m = arr_arr_union(aa, k, cc, m, bb);
	  pp = 0;
	}
	intl++;
      }
    }
    itva++;
    itvb--;
  }
  if(pp == 0){
    for(i = 0; i < m; i++){
      jp->ileft = bb[i].ileft;
      jp->iright = bb[i].iright;
      jp++;
    }
  } else {
    for(i = 0; i < m; i++){
      jp->ileft = cc[i].ileft;
      jp->iright = cc[i].iright;
      jp++;
    }
  }
  return(m);
}
/************************************************************************
*   Function   : line_struct_dil					*
*   Date       : Mon Nov  3 07:35:06 1997				*
*************************************************************************
*   Synopsis   :							*
*   Returns    :							*
*   Parameters :							*
*   Global refs:							*
************************************************************************/

static int line_struct_dil(
  WlzIntervalLine	*itvl,
  WlzInterval		*jntl,
  WlzInterval		*ll)
{
  WlzInterval *intl = itvl->intvs;
  WlzInterval *ptr;
  int i, curr, n = itvl->nintvs; 
  ptr = intl + 1;
  curr = jntl->iright - jntl->ileft + 1;
  ll->ileft = jntl->ileft + intl->ileft;
  i = 1;
  while(n-- > 1){
    if(ptr->ileft > intl->iright + curr){
      ll->iright = intl->iright + jntl->iright;
      ll++;
      ll->ileft = jntl->ileft + ptr->ileft;
      i++;
    } 
    intl++;
    ptr++;
  }
  ll->iright = intl->iright + jntl->iright;
  return(i);
}

/************************************************************************
*   Function   : arr_arr_union						*
*   Date       : Mon Nov  3 07:35:40 1997				*
*************************************************************************
*   Synopsis   :							*
*   Returns    :							*
*   Parameters :							*
*   Global refs:							*
************************************************************************/
/*
 * Find the union intervals between two lines.
 * One set of m intervals is in aa.
 * The other n intervals is in bb.
 * Length is the result number of individual intervals
 * in array cc.
 */

static int arr_arr_union(
  WlzInterval	*aa,
  int		m,
  WlzInterval	*bb,
  int		n,
  WlzInterval	*cc)
{
  register int flag = 0;
  WlzInterval *tmp = cc;

  if(m == 1 && n == 1){
    if(aa->iright + 1 < bb->ileft){
      cc->ileft = aa->ileft;
      cc->iright = aa->iright;
      cc++;
      cc->ileft = bb->ileft;
      cc->iright = bb->iright;
      return(2);
    } else if(bb->iright + 1 < aa->ileft){
      cc->ileft = bb->ileft;
      cc->iright = bb->iright;
      cc++;
      cc->ileft = aa->ileft;
      cc->iright = aa->iright;
      return(2);
    } else {
      cc->ileft = (aa->ileft < bb->ileft) ?
	aa->ileft : bb->ileft;
      cc->iright = (aa->iright > bb->iright) ?
	aa->iright : bb->iright;
      return(1);
    }
  }
  while(m > 0 && n > 0){
    if(aa->iright + 1 < bb->ileft){
      if(flag == 0){
	cc->ileft = aa->ileft;
	cc->iright = aa->iright;
      } else flag = 0;
      cc++;
      aa++;
      m--;
    } else if(bb->iright + 1 < aa->ileft){
      if(flag == 0){
	cc->ileft = bb->ileft;
	cc->iright = bb->iright;
      } else flag = 0;
      bb++;
      cc++;
      n--;
    } else {
      if(flag == 0)
	cc->ileft = (aa->ileft < bb->ileft) ?
	  aa->ileft : bb->ileft;
      if(aa->iright < bb->iright){
	cc->iright = bb->iright;
	aa++;
	m--;
      } else {
	cc->iright = aa->iright;
	bb++;
	n--;
      }
      flag = 1;
    }
  }
  if(flag == 1){
    n--;
    m--;
    aa++;
    bb++;
    cc++;
  }
  while(n-- > 0){
    cc->ileft = bb->ileft;
    cc->iright = bb->iright;
    cc++;
    bb++;
  }
  while(m-- > 0){
    cc->ileft = aa->ileft;
    cc->iright = aa->iright;
    cc++;
    aa++;
  }
  return((int)(cc-tmp));
}

/************************************************************************
*   Function   : WlzStructDilation3d					*
*   Date       : Mon Nov  3 07:34:47 1997				*
*************************************************************************
*   Synopsis   :							*
*   Returns    :							*
*   Parameters :							*
*   Global refs:							*
************************************************************************/

static WlzObject *WlzStructDilation3d(
  WlzObject 	*obj,
  WlzObject	*structElm,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzObject	*obj1, *obj2, *obj3;
  WlzObject	**objList;
  WlzDomain	domain, *domains, *domains1, *domains2;
  WlzValues	values;
  int		i, j, p, plane1, lastpl, nStructPlanes;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* the object is definitely 3D but the domain needs checking */
  if( obj->domain.p == NULL ){
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else {
    switch( obj->domain.p->type ){
    case WLZ_PLANEDOMAIN_DOMAIN:
      break;

    case WLZ_EMPTY_DOMAIN:
      return WlzMakeEmpty(dstErr);

    default:
      errNum = WLZ_ERR_DOMAIN_TYPE;
      break;
    }
  }

  /* check the structuring element, a 2D element is applied to each
     plane by creating a temporary 3D object */
  if( errNum == WLZ_ERR_NONE ){
    if( structElm == NULL ){
      errNum = WLZ_ERR_OBJECT_NULL;
    }
    else {
      switch( structElm->type ){
      case WLZ_2D_DOMAINOBJ:
	/* check the domain */
	if( structElm->domain.i == NULL ){
	  errNum = WLZ_ERR_DOMAIN_NULL;
	}
	else if( structElm->domain.i->type == WLZ_EMPTY_DOMAIN ){
	  values.core = NULL;
	  return WlzMakeMain(obj->type, obj->domain, values,
			     NULL, NULL, dstErr);
	}
	if( domain.p = WlzMakePlaneDomain(WLZ_PLANEDOMAIN_DOMAIN,
					  0, 0,
					  structElm->domain.i->line1,
					  structElm->domain.i->lastln,
					  structElm->domain.i->kol1,
					  structElm->domain.i->lastkl,
					  &errNum) ){
	  domain.p->domains[0] = WlzAssignDomain(structElm->domain, NULL);
	  values.core = NULL;
	  if( obj1 = WlzMakeMain(WLZ_3D_DOMAINOBJ, domain, values,
				 NULL, NULL, &errNum) ){
	    rtnObj = WlzStructDilation3d(obj, obj1, &errNum);
	    WlzFreeObj(obj1);
	  }
	  else {
	    WlzFreePlaneDomain(domain.p);
	  }
	}
	break;
	
      case WLZ_3D_DOMAINOBJ:
	/* check the domain and type */
	if( structElm->domain.p == NULL ){
	  errNum = WLZ_ERR_DOMAIN_NULL;
	}
	else {
	  switch( structElm->domain.p->type ){
	  case WLZ_PLANEDOMAIN_DOMAIN:
	    break;

	  case WLZ_EMPTY_DOMAIN:
	    return WlzMakeMain(obj->type, obj->domain, values,
			       NULL, NULL, dstErr);

	  default:
	    errNum = WLZ_ERR_DOMAIN_TYPE;
	    break;
	  }
	}
	break;

      case WLZ_TRANS_OBJ:
	return WlzStructDilation3d(obj, structElm->values.obj, dstErr);

      case WLZ_EMPTY_OBJ:
	values.core = NULL;
	return WlzMakeMain(obj->type, obj->domain, values,
			   NULL, NULL, dstErr);

      default:
	errNum = WLZ_ERR_OBJECT_TYPE;
      }
    }
  }

  /* this far with no errors and no rtnObj then the object and structuring
     element are 3D and have non-null and non-empty plane domains.
     We assume that the structuring element and object are standardised
     */
  if( (errNum == WLZ_ERR_NONE) && !rtnObj ){
    /* make a new planedomain with planes eroded and shifted by the
       planes of the structuring element, leave the lines and kols 
       to be sorted out by WlzStandardPlaneDomain */
    plane1 = obj->domain.p->plane1 + structElm->domain.p->plane1; 
    lastpl = obj->domain.p->lastpl + structElm->domain.p->lastpl;
    domain.p = WlzMakePlaneDomain(WLZ_PLANEDOMAIN_DOMAIN,
				  plane1, lastpl,
				  obj->domain.p->line1,
				  obj->domain.p->lastln,
				  obj->domain.p->kol1,
				  obj->domain.p->lastkl,
				  &errNum);
    domain.p->voxel_size[0] = obj->domain.p->voxel_size[0];
    domain.p->voxel_size[1] = obj->domain.p->voxel_size[1];
    domain.p->voxel_size[2] = obj->domain.p->voxel_size[2];
    domains = domain.p->domains;
    domains1 = obj->domain.p->domains;
    domains2 = structElm->domain.p->domains;
    values.core = NULL;
    nStructPlanes = structElm->domain.p->lastpl -
      structElm->domain.p->plane1 + 1;
    objList = (WlzObject **) AlcMalloc(sizeof(WlzObject *) * nStructPlanes);
    for(p=plane1; p <= lastpl; p++, domains++){
      for(i=0; i < nStructPlanes; i++){
	/* get the corresponding object plane */
	if(((p-i-structElm->domain.p->plane1) >= obj->domain.p->plane1) &&
	   ((p-i-structElm->domain.p->plane1) <= obj->domain.p->lastpl) &&
	   domains1[p-i-structElm->domain.p->plane1
		    -obj->domain.p->plane1].core ){
	  obj1 = WlzMakeMain(WLZ_2D_DOMAINOBJ,
			     domains1[p-i-structElm->domain.p->plane1
				      -obj->domain.p->plane1],
			     values, NULL, NULL, NULL);
	}
	else {
	  obj1 = WlzMakeEmpty(NULL);
	}
	obj1 = WlzAssignObject(obj1, NULL);
	/* get the structuring element plane */
	if( domains2[i].core ){
	  obj2 = WlzMakeMain(WLZ_2D_DOMAINOBJ, domains2[i], values,
			     NULL, NULL, NULL);
	}
	else {
	  obj2 = WlzMakeEmpty(NULL);
	}
	obj2 = WlzAssignObject(obj2, NULL);
	objList[i] = WlzAssignObject(WlzStructDilation(obj1, obj2, NULL),
				     NULL);
      }
      obj3 = WlzUnionN(nStructPlanes, objList, 0, &errNum);
      if( (obj3 == NULL) || (obj3->type == WLZ_EMPTY_OBJ) ){
	(*domains).core = NULL;
      }
      else {
	*domains = WlzAssignDomain(obj3->domain, NULL);
      }
      WlzFreeObj(obj1);
      WlzFreeObj(obj2);
      if( obj3 ){
	WlzFreeObj(obj3);
      }
      for(i=0; i < nStructPlanes; i++){
	if( objList[i] ){
	  WlzFreeObj(objList[i]);
	}
      }
    }
    AlcFree((void *) objList);
    WlzStandardPlaneDomain(domain.p, NULL);
    values.core = NULL;
    rtnObj = WlzMakeMain(WLZ_3D_DOMAINOBJ, domain, values,
			 NULL, NULL, &errNum);
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}

