#pragma ident "MRC HGU $Id$"
/***********************************************************************
* Project:      Woolz
* Title:        WlzReadObj.c
* Date:         March 1999
* Author:       Richard Baldock
* Copyright:	1999 Medical Research Council, UK.
*		All rights reserved.
* Address:	MRC Human Genetics Unit,
*		Western General Hospital,
*		Edinburgh, EH4 2XU, UK.
* Purpose:      Reads a Woolz object from a file.
* $Revision$
* Maintenance:	Log changes below, with most recent at top of list.
************************************************************************/
#include <stdlib.h>
#include <string.h>

#include <Wlz.h>

/* prototypes of static procedures defined below
 */

static WlzIntervalDomain 	*WlzReadIntervalDomain(FILE *fp,
						       WlzErrorNum *);
static WlzPlaneDomain 		*WlzReadPlaneDomain(FILE *fp,
						    WlzErrorNum *);
static WlzErrorNum		WlzReadGreyValues(FILE *fp,
						  WlzObject *obj);
static WlzErrorNum		WlzReadRectVtb(FILE 		*fp,
					       WlzObject 	*obj,
					       WlzObjectType type);
static WlzErrorNum		WlzReadVoxelValues(FILE      *fp,
						   WlzObject *obj);
static WlzSimpleProperty 	*WlzReadPropertyList(FILE *fp,
						     WlzErrorNum *);
static WlzPolygonDomain 	*WlzReadPolygon(FILE *fp,
						WlzErrorNum *);
static WlzBoundList 		*WlzReadBoundList(FILE *fp,
						  WlzErrorNum *);
static WlzIRect 			*WlzReadRect(FILE *fp,
					     WlzErrorNum *);
static WlzObject 		*WlzReadVector(FILE		*fp,
					       WlzObjectType	type,
					       WlzErrorNum *);
static WlzObject 		*WlzReadPoint(FILE		*fp,
					      WlzObjectType	type,
					      WlzErrorNum *);
static WlzHistogramDomain 	*WlzReadHistogramDomain(FILE *fp,
							WlzErrorNum *);
static WlzObject 		*WlzReadCompoundA(FILE		*fp,
						  WlzObjectType	type,
						  WlzErrorNum *);
static WlzAffineTransform 	*WlzReadAffineTransform(FILE *fp,
							WlzErrorNum *);
static WlzWarpTrans 		*WlzReadWarpTrans(FILE *fp,
						  WlzErrorNum *);
static WlzFMatchObj 		*WlzReadFMatchObj(FILE *fp,
						  WlzErrorNum *);
static Wlz3DWarpTrans 		*WlzRead3DWarpTrans(FILE *fp,
						    WlzErrorNum *);

/* a set of functions to convert from VAX to SUN byte ordering
   in the future these should be replaced by calls using XDR procedures
   */

/************************************************************************
*   Function   : getword						*
*   Synopsis   : get the next word (int) from the input stream		*
*   Returns    : int:		value of next word on the input stream	*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -                                                      *
************************************************************************/

static int getword(FILE *fp)
{
  char cin[4], cout[4];

  fread(cin,sizeof(char),4,fp);
#if defined (__sparc) || defined (__mips)
  cout[0] = cin[3];
  cout[1] = cin[2];
  cout[2] = cin[1];
  cout[3] = cin[0];
#endif /* __sparc || __mips */
#if defined (__x86) || defined (__alpha)
  cout[0] = cin[0];
  cout[1] = cin[1];
  cout[2] = cin[2];
  cout[3] = cin[3];
#endif /* __x86 || __alpha */
  return(*((int *) &cout[0]));
}

/************************************************************************
*   Function   : getshort						*
*   Synopsis   : get the next short word (int) from the input stream	*
*   Returns    : int:		value of next word on the input stream	*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -                                                      *
************************************************************************/

static int getshort(FILE *fp)
{
  char cin[2], cout[2];

  fread(cin,sizeof(char),2,fp);
#if defined (__sparc) || defined (__mips)
  cout[0] = cin[1];
  cout[1] = cin[0];
#endif /* __sparc || __mips */
#if defined (__x86) || defined (__alpha)
  cout[0] = cin[0];
  cout[1] = cin[1];
#endif /* __x86 || __alpha */
  return((int) *((short *) &cout[0]));
}

/************************************************************************
*   Function   : getfloat						*
*   Synopsis   : get the next float from the input stream		*
*   Returns    : float:		value of next float on the input stream	*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -                                                      *
************************************************************************/

static float getfloat(FILE *fp)
{
  char cin[4], cout[4];

  fread(cin,sizeof(char),4,fp);
#if defined (__sparc) || defined (__mips)
  cout[0] = cin[1] - 1;
  cout[1] = cin[0];
  cout[2] = cin[3];
  cout[3] = cin[2];
#endif /* __sparc || __mips */
#if defined (__x86) || defined (__alpha)
  cout[3] = cin[1] - 1;
  cout[2] = cin[0];
  cout[1] = cin[3];
  cout[0] = cin[2];
#endif /* __x86 || __alpha */
  return(*((float *) &cout[0]));
}

/************************************************************************
*   Function   : getdouble						*
*   Synopsis   : get the next double from the input stream		*
*   Returns    : double:	value of next double on the input stream*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -                                                      *
************************************************************************/

static double getdouble(FILE *fp)
{
  char cin[8], cout[8];

  fread(cin,sizeof(char),8,fp);
#if defined (__sparc) || defined (__mips)
  cout[0] = cin[7];
  cout[1] = cin[6];
  cout[2] = cin[5];
  cout[3] = cin[4];
  cout[4] = cin[3];
  cout[5] = cin[2];
  cout[6] = cin[1];
  cout[7] = cin[0];
#endif /* __sparc || __mips */
#if defined (__x86) || defined (__alpha)
  cout[3] = cin[7];
  cout[2] = cin[6];
  cout[1] = cin[5];
  cout[0] = cin[4];
  cout[7] = cin[3];
  cout[6] = cin[2];
  cout[5] = cin[1];
  cout[4] = cin[0];
#endif /* __x86 || __alpha */
  return(*((double *) &cout[0]));
}

/************************************************************************
*   Function   : WlzReadObj						*
*   Synopsis   : reads a woolz object from the given input stream	*
*   Returns    : WlzObject *:	non-NULL - successful read		*
*				NULL - end of file, incomplete read or	*
*				memory allocation error			*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: char *err_str:	static string buffer for error message	*
************************************************************************/

WlzObject *WlzReadObj(FILE *fp, WlzErrorNum *dstErr)
{
  WlzObjectType		type;
  WlzObject 		*obj;
  WlzDomain		domain;
  WlzValues		values;

  Wlz3DWarpTrans	*wtrans3d;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  /* check the stream pointer */
  if( fp == NULL ){
    errNum = WLZ_ERR_PARAM_NULL;
  }
  else if( feof(fp) != 0 ){
    errNum = WLZ_ERR_READ_EOF;
  }
  else {
    /* initialise the obj pointer and domain and values unions */
    obj = NULL;
    domain.core = NULL;
    values.core = NULL;

    type = (WlzObjectType) getc(fp);

    switch( type ){

    case (WlzObjectType) EOF:
      errNum = WLZ_ERR_READ_EOF;
      break;

    case WLZ_NULL:	/* signifies no more objects - not a true error */
      errNum = WLZ_ERR_EOO;
      break;

    case WLZ_EMPTY_OBJ:
      obj =  WlzMakeMain(WLZ_EMPTY_OBJ, domain, values, NULL,
			 NULL, &errNum);
      break;

    case WLZ_2D_DOMAINOBJ:
      if((domain.i = WlzReadIntervalDomain(fp, &errNum)) &&
	 (obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum)) )
      {
	if( (errNum = WlzReadGreyValues(fp, obj)) == WLZ_ERR_NONE ){
	  obj->plist = WlzAssignProperty(WlzReadPropertyList(fp, NULL),
					 NULL);
	}
	else {
	  WlzFreeObj( obj );
	  obj = NULL;
	}
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      if((domain.p = WlzReadPlaneDomain(fp, &errNum)) &&
	 (obj = WlzMakeMain( type, domain, values, NULL, NULL, &errNum)) )
      {
	if( (errNum = WlzReadVoxelValues(fp, obj)) == WLZ_ERR_NONE ){
	  obj->plist = WlzAssignProperty(WlzReadPropertyList(fp, NULL),
					 NULL);
	}
	else {
	  /* attempt to return a partial object */
/*	  WlzFreeObj( obj );
	  obj = NULL;*/
	}
      }
      break;

    case WLZ_TRANS_OBJ:
      if( domain.t = WlzReadAffineTransform( fp, &errNum ) ){
	if( values.obj = WlzReadObj( fp, &errNum ) ){
	  if( obj = WlzMakeMain(WLZ_TRANS_OBJ, domain, values,
				NULL, NULL, &errNum) ){
	    obj->plist = WlzAssignProperty(WlzReadPropertyList(fp, NULL),
					   NULL);
	  } else {
	    WlzFreeAffineTransform(domain.t);
	    WlzFreeObj(values.obj);
	  }
	}
	else {
	  WlzFreeAffineTransform(domain.t);
	}
      }
      break;

    case WLZ_3D_WARP_TRANS:
      if( wtrans3d = WlzRead3DWarpTrans(fp, &errNum) ){
	wtrans3d->plist = WlzAssignProperty(WlzReadPropertyList(fp, NULL),
					    NULL);
      }
      obj = (WlzObject *) wtrans3d;
      break;

    case WLZ_2D_POLYGON:
      if( domain.poly = WlzReadPolygon(fp, &errNum) ){
	obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum);
      }
      break;

    case WLZ_BOUNDLIST:
      if( domain.b = WlzReadBoundList(fp, &errNum) ){
	obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum);
      }
      break;

    case WLZ_HISTOGRAM:
      if( domain.hist = WlzReadHistogramDomain(fp, &errNum) ){
	obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum);
      }
      break;

    case WLZ_RECTANGLE:
      if( domain.r = WlzReadRect(fp, &errNum) ){
	obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum);
      }
      break;

    case WLZ_VECTOR_INT:
    case WLZ_VECTOR_FLOAT:
      obj = (WlzObject *) WlzReadVector(fp, type, &errNum);
      break;

    case WLZ_POINT_INT:
    case WLZ_POINT_FLOAT:
      obj = (WlzObject *) WlzReadPoint(fp, type, &errNum);
      break;

    case WLZ_AFFINE_TRANS:
      if( domain.t = WlzReadAffineTransform(fp, &errNum) ){
	obj = WlzMakeMain(type, domain, values, NULL, NULL, &errNum);
      }
      break;

    case WLZ_WARP_TRANS:
      obj = (WlzObject *) WlzReadWarpTrans(fp, &errNum);
      break;

    case WLZ_FMATCHOBJ:
      obj = (WlzObject *) WlzReadFMatchObj(fp, &errNum);
      break;

    case WLZ_COMPOUND_ARR_1:
    case WLZ_COMPOUND_ARR_2:
      obj = (WlzObject *) WlzReadCompoundA(fp, type, &errNum);
      break;

    case WLZ_PROPERTY_OBJ:
      obj = WlzMakeMain(type, domain, values,
			WlzReadPropertyList(fp, NULL), NULL, &errNum);
      break;

      /* orphans and not yet implemented object types for I/O */
    case WLZ_CONV_HULL:
    case WLZ_3D_POLYGON:
    case WLZ_CONVOLVE_INT:
    case WLZ_CONVOLVE_FLOAT:
    case WLZ_DISP_FRAME:
    case WLZ_DISP_GRID:
    case WLZ_DISP_FRAMEX:
    case WLZ_TEXT:
    case WLZ_COMPOUND_LIST_1:
    case WLZ_COMPOUND_LIST_2:
    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}

/************************************************************************
*   Function   : WlzIntervaldomain					*
*   Synopsis   : reads a WlzIntervalDomain from the given input stream	*
*   Returns    : WlzIntervalDomain *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzIntervalDomain *WlzReadIntervalDomain(FILE *fp,
						WlzErrorNum *dstErr)
{
  WlzObjectType		type;
  int			i, l, l1, ll, k1, kl, nints;
  WlzIntervalDomain	*idmn=NULL;	
  WlzIntervalLine 	*ivln;
  WlzInterval 		*itvl0,
  			*itvl;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  /* read the type, currently WriteObj will write '\0' given a
     NULL pointer so we do the same here setting the error to be
     WLZ_ERR_EOO to distinguish it from an EOF error */
  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == WLZ_NULL ){
    errNum = WLZ_ERR_EOO;
  }

  if( errNum == WLZ_ERR_NONE ){
    l1 = getword(fp);
    ll = getword(fp);
    k1 = getword(fp);
    kl = getword(fp);
    if( feof(fp) != 0 ){
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    else {
      idmn = WlzMakeIntervalDomain(type, l1, ll, k1, kl, &errNum);
    }
  }

  if( errNum == WLZ_ERR_NONE ){
    switch (type) {

    case WLZ_INTERVALDOMAIN_INTVL:
      nints = 0;
      ivln = idmn->intvlines;
      for (l=l1; l<=ll; l++) {
	ivln->nintvs = getword(fp);
	nints += ivln->nintvs;
	ivln++;
      }
      if( feof(fp) != 0 ){
	WlzFreeIntervalDomain(idmn);
	idmn = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
	break;
      }

      if( nints == 0 ){
	/* curious case of a no-intervals domain */
	WlzFreeIntervalDomain(idmn);
	idmn = NULL;
	errNum = WLZ_ERR_EOO;
	break;
      }
      else if( (itvl0 = (WlzInterval *)
	   AlcMalloc(nints * sizeof(WlzInterval))) == NULL){
	WlzFreeIntervalDomain(idmn);
	idmn = NULL;
	errNum = WLZ_ERR_MEM_ALLOC;
	break;
      }
      itvl = itvl0;
      ivln = idmn->intvlines;
      idmn->freeptr = WlzPushFreePtr(idmn->freeptr, (void *)itvl0, NULL);

      for (i=0; i<nints; i++,itvl++) {
	itvl->ileft = getword(fp);
	itvl->iright = getword(fp);
      }

      itvl = itvl0;

      if (feof(fp) != 0){
	WlzFreeIntervalDomain(idmn);
	idmn = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
	break;
      }
    
      for (l=l1; l<=ll; l++) {
	nints = ivln->nintvs;
	errNum = WlzMakeInterval(l, idmn, nints, itvl);
	ivln++;
	itvl += nints;
      }
      errNum = WlzStandardIntervalDomain(idmn);
      break;

    case WLZ_INTERVALDOMAIN_RECT:
      break;

    default:
      /* this can't happen because the domain type has been checked
	 by WlzMakeIntervalDomain */
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(idmn);
}

/************************************************************************
*   Function   : WlzReadPlaneDomain					*
*   Synopsis   : reads a WlzPlaneDomain from the given input stream	*
*   Returns    : WlzPlaneDomain *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzPlaneDomain *WlzReadPlaneDomain(FILE *fp,
					  WlzErrorNum *dstErr)
{
  WlzObjectType		type;
  WlzDomain		domain, *domains;
  WlzPlaneDomain	*planedm=NULL;
  int			i, nplanes;
  int			p1, pl, l1, ll, k1, kl;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == WLZ_NULL ){
    errNum = WLZ_ERR_EOO;
  }
  else if( type == (WlzObjectType) 2 ){
    /* some old object files have this old value - now converted
       silently but we must continue to read these files */
    type = WLZ_PLANEDOMAIN_DOMAIN;
  }

  if( errNum == WLZ_ERR_NONE ){
    p1 = getword(fp);
    pl = getword(fp);
    l1 = getword(fp);
    ll = getword(fp);
    k1 = getword(fp);
    kl = getword(fp);
    if (feof(fp) != 0){
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    else if( planedm = WlzMakePlaneDomain(type, p1, pl, l1, ll, k1, kl,
					  &errNum) ){
      domains = planedm->domains;
      (planedm->voxel_size)[0] = getfloat(fp);
      (planedm->voxel_size)[1] = getfloat(fp);
      (planedm->voxel_size)[2] = getfloat(fp);
      nplanes = pl - p1 + 1;

      /* object format includes redundant plane positions -
	 read and discard */
      for(i=0; i < nplanes; i++){
	(void) getfloat(fp);
      }
      if (feof(fp) != 0){
	WlzFreePlaneDomain(planedm);
	planedm = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
    }
  }

  if( errNum == WLZ_ERR_NONE ){
    switch (type) {

    case WLZ_PLANEDOMAIN_DOMAIN:
      for(i=0; i < nplanes; i++, domains++){
	if( domain.i = WlzReadIntervalDomain(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break;

    case WLZ_PLANEDOMAIN_POLYGON:
      for(i=0; i < nplanes; i++, domains++){
	if( domain.poly = WlzReadPolygon(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break;

    case WLZ_PLANEDOMAIN_BOUNDLIST:
      for(i=0; i < nplanes; i++, domains++){
	if( domain.b = WlzReadBoundList(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break;

    case WLZ_PLANEDOMAIN_HISTOGRAM:
      for(i=0; i < nplanes; i++, domains++){
	if( domain.hist = WlzReadHistogramDomain(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break;

    case WLZ_PLANEDOMAIN_AFFINE:
      for (i=0; i< nplanes ; i++, domains++){
	if( domain.t = WlzReadAffineTransform(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break ;

    case WLZ_PLANEDOMAIN_WARP:
      for (i=0; i< nplanes ; i++, domains++){
	if( domain.wt = WlzReadWarpTrans(fp, &errNum) ){
	  *domains = WlzAssignDomain(domain, NULL);
	} else if( errNum == WLZ_ERR_EOO ){
	  errNum = WLZ_ERR_NONE;
	} else {
	  break;
	}
      }
      break ;

    default:
      /* this can't happen because the domain type has been checked
	 by WlzMakeIntervalDomain */
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(planedm);
}

/************************************************************************
*   Function   : WlzReadGreyValues					*
*   Synopsis   : reads a woolz grey-value table				*
*   Returns    : int:		error code, WLZ_ERR_NONE for successful	*
*				completion				*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObject *obj: object defining the domain of the grey	*
*				values.					*
*   Global refs: -							*
************************************************************************/

static WlzErrorNum WlzReadGreyValues(FILE *fp,
				     WlzObject *obj)
{
  WlzObjectType		type;
  WlzGreyType		gtype;
  WlzIntervalWSpace 	iwsp;
  WlzValues		values;
  WlzGreyType		packing;
  int 			kstart, l1, ll, k1;
  int 			i;
  WlzPixelV 		backgrnd;
  WlzGreyP		v, g;
  size_t		table_size;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF )
  {
    return WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == WLZ_NULL )
  {
    obj->values.core = NULL;
    return WLZ_ERR_NONE;
  }
  gtype = (WlzGreyType) type;

  /*
   * The "type" read from disc only coded the pixel type.
   * The shape type is determined by the object domain.
   * For the moment, choice is between standard ragged-rectangle
   * or true rectangle.
   */
  if( obj->domain.core == NULL ){
    return WLZ_ERR_DOMAIN_NULL;
  }
    
  switch( obj->domain.core->type ){

  case WLZ_INTERVALDOMAIN_INTVL:
    type = WlzGreyTableType(WLZ_GREY_TAB_RAGR, gtype, &errNum);
    break;
    
  case WLZ_INTERVALDOMAIN_RECT:
    type = WlzGreyTableType(WLZ_GREY_TAB_RECT, gtype, &errNum);
    break;

  default:
    obj->values.core = NULL;
    return( WLZ_ERR_DOMAIN_TYPE );

  }
  if( errNum != WLZ_ERR_NONE ){
    return errNum;
  }
    
  l1 = obj->domain.i->line1;
  ll = obj->domain.i->lastln;
  k1 = obj->domain.i->kol1;
  backgrnd.type = gtype;
  switch (type) {

  case WLZ_VALUETABLE_RAGR_INT:
    packing = (WlzGreyType) getc(fp);
    backgrnd.v.inv = getword(fp);

    /* create the value table */
    if( (values.v = WlzMakeValueTb(type, l1, ll, k1,
				   backgrnd, obj, &errNum)) == NULL ){
      return errNum;
    }
    values.v->width = obj->domain.i->lastkl - k1 + 1;
    obj->values = WlzAssignValues(values, NULL);

    /* allocate space for the pixel values, preset to background value */
    table_size = WlzLineArea(obj, NULL) * sizeof(int);
    if( (v.inp = (int *) AlcMalloc(table_size)) == NULL){
      WlzFreeValueTb(values.v);
      obj->values.v = NULL;
      return WLZ_ERR_MEM_ALLOC;
    }
    memset((void *) v.inp, backgrnd.v.inv, table_size);
    values.v->freeptr = WlzPushFreePtr(values.v->freeptr, (void *)v.inp,
    				       NULL);

    if( (errNum = WlzInitRasterScan(obj, &iwsp,
    				    WLZ_RASTERDIR_ILIC)) == WLZ_ERR_NONE ){
      while((errNum = WlzNextInterval(&iwsp)) == WLZ_ERR_NONE) {
	if (iwsp.nwlpos){
	  kstart = iwsp.lftpos;
	}
	switch (packing) {
	case WLZ_GREY_INT:
	  g.inp = v.inp+iwsp.lftpos-kstart;
	  for (i=0; i<iwsp.colrmn; i++){
	    *g.inp++ = getword(fp);
	  }
	  break;
	case WLZ_GREY_SHORT:
	  g.inp = v.inp+iwsp.lftpos-kstart;
	  for (i=0; i<iwsp.colrmn; i++){
	    *g.inp++ = getshort(fp);
	  }
	  break;
	case WLZ_GREY_UBYTE:
	  g.inp = v.inp+iwsp.lftpos-kstart;
	  for (i=0; i<iwsp.colrmn; i++){
	    *g.inp++ = getc(fp);
	  }
	  break;
	}
	if (iwsp.intrmn == 0) {
	  (void) WlzMakeValueLine(values.v, iwsp.linpos, kstart,
				  iwsp.rgtpos, v.inp);
	  v.inp += (iwsp.rgtpos - kstart + 1);
	}
      }
      if (feof(fp) != 0){
	WlzFreeValueTb(values.v);
	obj->values.v = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
      else if( errNum == WLZ_ERR_EOO ){
	errNum = WLZ_ERR_NONE;
      }
    }
    return errNum;

  case WLZ_VALUETABLE_RAGR_SHORT:
    packing = (WlzGreyType) getc(fp);
    backgrnd.v.shv = getword(fp);

    /* create the value table */
    if( (values.v = WlzMakeValueTb(type, l1, ll, k1,
				   backgrnd, obj, &errNum)) == NULL ){
      return errNum;
    }
    values.v->width = obj->domain.i->lastkl - k1 + 1;
    obj->values = WlzAssignValues(values, NULL);

    /* allocate space for the pixel values, preset to background value */
    table_size = WlzLineArea(obj, NULL) * sizeof(short);
    if( (v.shp = (short *) AlcMalloc(table_size)) == NULL){
      WlzFreeValueTb(values.v);
      obj->values.v = NULL;
      return WLZ_ERR_MEM_ALLOC;
    }
    memset((void *) v.shp, (int) backgrnd.v.shv, table_size);
    values.v->freeptr = WlzPushFreePtr(values.v->freeptr, (void *)v.shp,
    					NULL);

    if( (errNum = WlzInitRasterScan(obj, &iwsp,
    				    WLZ_RASTERDIR_ILIC)) == WLZ_ERR_NONE ){
      while((errNum = WlzNextInterval(&iwsp)) == WLZ_ERR_NONE) {
	if (iwsp.nwlpos){
	  kstart = iwsp.lftpos;
	}
	switch (packing) {
	case WLZ_GREY_SHORT:
	  g.shp = v.shp+iwsp.lftpos-kstart;
	  for (i=0; i<iwsp.colrmn; i++){
	    *g.shp++ = getshort(fp);
	  }
	  break;
	case WLZ_GREY_UBYTE:
	  g.shp = v.shp+iwsp.lftpos-kstart;
	  for (i=0; i<iwsp.colrmn; i++){
	    *g.shp++ = getc(fp);
	  }
	  break;
	}
	if (iwsp.intrmn == 0) {
	  (void) WlzMakeValueLine(values.v, iwsp.linpos, kstart,
				  iwsp.rgtpos, v.inp);
	  v.shp += (iwsp.rgtpos - kstart + 1);
	}
      }
      if( feof(fp) != 0 ){
	WlzFreeValueTb(values.v);
	obj->values.v = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
      else if( errNum == WLZ_ERR_EOO ){
	errNum = WLZ_ERR_NONE;
      }
    }
    return errNum;

  case WLZ_VALUETABLE_RAGR_UBYTE:
    packing = (WlzGreyType) getc(fp);
    backgrnd.v.ubv = getword(fp);

    /* create the value table */
    if( (values.v = WlzMakeValueTb(type, l1, ll, k1,
				   backgrnd, obj, &errNum)) == NULL ){
      return errNum;
    }
    values.v->width = obj->domain.i->lastkl - k1 + 1;
    obj->values = WlzAssignValues(values, NULL);

    /* allocate space for the pixel values, preset to background value */
    table_size = WlzLineArea(obj, NULL) * sizeof(UBYTE);
    if( (v.ubp = (UBYTE *) AlcMalloc(table_size)) == NULL){
      WlzFreeValueTb(values.v);
      obj->values.v = NULL;
      return WLZ_ERR_MEM_ALLOC;
    }
    memset((void *) v.ubp, (int) backgrnd.v.ubv, table_size);
    values.v->freeptr = WlzPushFreePtr(values.v->freeptr, (void *)v.ubp, NULL);

    if( (errNum = WlzInitRasterScan(obj, &iwsp,
    				    WLZ_RASTERDIR_ILIC)) == WLZ_ERR_NONE ){
      while((errNum = WlzNextInterval(&iwsp)) == WLZ_ERR_NONE) {
	if (iwsp.nwlpos){
	  kstart = iwsp.lftpos;
	}
	g.ubp = v.ubp+iwsp.lftpos-kstart;
	for (i=0; i<iwsp.colrmn; i++){
	  *g.ubp++ = getc(fp);
	}
	if (iwsp.intrmn == 0) {
	  (void) WlzMakeValueLine(values.v, iwsp.linpos, kstart,
				  iwsp.rgtpos, v.inp);
	  v.ubp += (iwsp.rgtpos - kstart + 1);
	}
      }
      if (feof(fp) != 0){
	WlzFreeValueTb(values.v);
	obj->values.v = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
      else if( errNum == WLZ_ERR_EOO ){
	errNum = WLZ_ERR_NONE;
      }
    }
    return( WLZ_ERR_NONE );

  case WLZ_VALUETABLE_RAGR_FLOAT:
    packing = (WlzGreyType) getc(fp);
    backgrnd.v.flv = getfloat(fp);

    /* create the value table */
    if( (values.v = WlzMakeValueTb(type, l1, ll, k1,
				   backgrnd, obj, &errNum)) == NULL ){
      return errNum;
    }
    values.v->width = obj->domain.i->lastkl - k1 + 1;
    obj->values = WlzAssignValues(values, NULL);

    /* allocate space for the pixel values, preset to background */
    table_size = WlzLineArea(obj, NULL) * sizeof(float);
    if( (v.flp = (float *) AlcMalloc(table_size)) == NULL){
      WlzFreeValueTb(values.v);
      obj->values.v = NULL;
      return WLZ_ERR_MEM_ALLOC;
    }
    memset((void *) v.flp, (int) backgrnd.v.flv, table_size);
    values.v->freeptr = WlzPushFreePtr(values.v->freeptr, (void *)v.flp,
    					NULL);

    if( (errNum = WlzInitRasterScan(obj, &iwsp,
    				    WLZ_RASTERDIR_ILIC)) == WLZ_ERR_NONE ){
      while((errNum = WlzNextInterval(&iwsp)) == WLZ_ERR_NONE) {
	if (iwsp.nwlpos){
	  kstart = iwsp.lftpos;
	}
	g.flp = v.flp+iwsp.lftpos-kstart;
	for (i=0; i<iwsp.colrmn; i++){
	  *g.flp++ = getfloat(fp);
	}
	if (iwsp.intrmn == 0) {
	  (void) WlzMakeValueLine(values.v, iwsp.linpos, kstart,
				  iwsp.rgtpos, v.inp);
	  v.flp += (iwsp.rgtpos - kstart + 1);
	}
      }
      if (feof(fp) != 0){
	WlzFreeValueTb(values.v);
	obj->values.v = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
      else if( errNum == WLZ_ERR_EOO ){
	errNum = WLZ_ERR_NONE;
      }
    }
    return errNum;

  case WLZ_VALUETABLE_RAGR_DOUBLE:
    packing = (WlzGreyType) getc(fp);
    backgrnd.v.dbv = getdouble(fp);

    /* create the value table */
    if( (values.v = WlzMakeValueTb(type, l1, ll, k1,
				   backgrnd, obj, &errNum)) == NULL ){
      return errNum;
    }
    values.v->width = obj->domain.i->lastkl - k1 + 1;
    obj->values = WlzAssignValues(values, NULL);

    /* allocate space for the pixel values, preset to background */
    table_size = WlzLineArea(obj, NULL) * sizeof(double);
    if( (v.dbp = (double *) AlcMalloc(table_size)) == NULL){
      WlzFreeValueTb(values.v);
      obj->values.v = NULL;
      return WLZ_ERR_MEM_ALLOC;
    }
    memset((void *) v.dbp, (int) backgrnd.v.dbv, table_size);
    values.v->freeptr = WlzPushFreePtr(values.v->freeptr, (void *)v.dbp,
    					NULL);

    if( (errNum = WlzInitRasterScan(obj, &iwsp,
    				    WLZ_RASTERDIR_ILIC)) == WLZ_ERR_NONE ){
      while((errNum = WlzNextInterval(&iwsp)) == WLZ_ERR_NONE) {
	if (iwsp.nwlpos){
	  kstart = iwsp.lftpos;
	}
	g.dbp = v.dbp+iwsp.lftpos-kstart;
	for (i=0; i<iwsp.colrmn; i++){
	  *g.dbp++ = getdouble(fp);
	}
	if (iwsp.intrmn == 0) {
	  (void) WlzMakeValueLine(values.v, iwsp.linpos, kstart,
				  iwsp.rgtpos, v.inp);
	  v.dbp += (iwsp.rgtpos - kstart + 1);
	}
      }
      if (feof(fp) != 0){
	WlzFreeValueTb(values.v);
	obj->values.v = NULL;
	errNum = WLZ_ERR_READ_INCOMPLETE;
      }
      else if( errNum == WLZ_ERR_EOO ){
	errNum = WLZ_ERR_NONE;
      }
    }
    return errNum;

  case WLZ_VALUETABLE_RECT_INT:
  case WLZ_VALUETABLE_RECT_SHORT:
  case WLZ_VALUETABLE_RECT_UBYTE:
  case WLZ_VALUETABLE_RECT_FLOAT:
  case WLZ_VALUETABLE_RECT_DOUBLE:
    return WlzReadRectVtb(fp, obj, type);

  default:
    /* this can't happen because the domain type has been checked
       by WlzGreyValuesTableType */
    return WLZ_ERR_VALUES_TYPE;
  }

}

/************************************************************************
*   Function   : WlzReadRectVtb						*
*   Synopsis   : reads a woolz rectangular grey table			*
*   Returns    : int:		error code, WLZ_ERR_NONE for success	*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObject *obj: object defining the domain of the grey	*
*				values.					*
*		 WlzObjectType type: grey table type - encodes greytype *
*   Global refs: -							*
************************************************************************/

static WlzErrorNum WlzReadRectVtb(FILE 		*fp,
				  WlzObject 	*obj,
				  WlzObjectType type)
{
  WlzGreyP		values;
  int 			i, num;
  WlzGreyType		packing;
  WlzIntervalDomain 	*idmn;
  WlzValues		vtb;
  WlzPixelV		bgd;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  packing = (WlzGreyType) getc(fp);
  if( (idmn = obj->domain.i) == NULL ){
    return( WLZ_ERR_DOMAIN_NULL );
  }    

  bgd.type = WlzGreyTableTypeToGreyType( type, NULL );
  bgd.v.inv = 0;
  if((vtb.r = WlzMakeRectValueTb(type, idmn->line1, idmn->lastln,
				 idmn->kol1, idmn->lastkl-idmn->kol1 + 1,
				 bgd, NULL, &errNum)) == NULL ){
    return errNum;
  }
  num = vtb.r->width * (vtb.r->lastln - vtb.r->line1 + 1);

  /* test on pixel type to read background */
  switch( WlzGreyTableTypeToGreyType( type, NULL ) ){
  case WLZ_GREY_INT:
    vtb.r->bckgrnd.v.inv = getword(fp);
    values.inp = (int *) AlcMalloc(num*sizeof(int));
    break;
  case WLZ_GREY_SHORT:
    vtb.r->bckgrnd.v.shv = getword(fp);
    values.shp = (short *) AlcMalloc(num*sizeof(short));
    break;
  case WLZ_GREY_UBYTE:
    vtb.r->bckgrnd.v.ubv = getword(fp);
    values.ubp = (UBYTE *) AlcMalloc(num*sizeof(UBYTE));
    break;
  case WLZ_GREY_FLOAT:
    vtb.r->bckgrnd.v.flv = getfloat(fp);
    values.flp = (float *) AlcMalloc(num*sizeof(float));
    break;
  case WLZ_GREY_DOUBLE:
    vtb.r->bckgrnd.v.dbv = getdouble(fp);
    values.dbp = (double *) AlcMalloc(num*sizeof(double));
    break;
  }

  if( values.inp == NULL ){
    WlzFreeValueTb(vtb.v);
    return WLZ_ERR_MEM_ALLOC;
  }
  vtb.r->freeptr = WlzPushFreePtr(vtb.r->freeptr, (void *)values.inp, NULL);
  vtb.r->values = values;
  obj->values = WlzAssignValues(vtb, NULL);

  switch( WlzGreyTableTypeToGreyType( type, NULL ) ) {

  case WLZ_GREY_INT:
    switch (packing) {
    case WLZ_GREY_INT:
      for (i=0 ; i<num ; i++)
	*values.inp++ = getword(fp);
      break;
    case WLZ_GREY_SHORT:
      for (i=0 ; i<num ; i++)
	*values.inp++ = getshort(fp);
      break;
    case WLZ_GREY_UBYTE:
      for (i=0 ; i<num ; i++)
	*values.inp++ = getc(fp);
      break;
    }
    break;

  case WLZ_GREY_SHORT:
    switch (packing) {
    case WLZ_GREY_SHORT:
      for (i=0 ; i<num ; i++)
	*values.shp++ = getshort(fp);
      break;
    case WLZ_GREY_UBYTE:
      for (i=0 ; i<num ; i++)
	*values.shp++ = getc(fp);
      break;
    }
    break;

  case WLZ_GREY_UBYTE:
    fread(values.ubp, sizeof(UBYTE), num, fp);
    break;

  case WLZ_GREY_FLOAT:
    for (i=0 ; i<num ; i++)
      *values.flp++ = getfloat(fp);
    break;

  case WLZ_GREY_DOUBLE:
    for (i=0 ; i<num ; i++)
      *values.dbp++ = getdouble(fp);
    break;

  }
  if( feof(fp) != 0 ){
    WlzFreeValueTb(vtb.v);
    obj->values.core = NULL;
    return  WLZ_ERR_READ_INCOMPLETE;
  }

  return WLZ_ERR_NONE;
}

/************************************************************************
*   Function   : WlzReadVoxelValues					*
*   Synopsis   : reads a woolz voxel value table			*
*   Returns    : int:		error code, WLZ_ERR_NONE for success	*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObject *obj: object defining the domain of the grey	*
*				values.					*
*   Global refs: -							*
************************************************************************/

static WlzErrorNum WlzReadVoxelValues(FILE 	*fp,
				      WlzObject *obj)
{
  WlzObjectType		type;
  int 			i, nplanes;
  WlzObject 		*tmpobj;
  WlzDomain 		*domains;
  WlzValues		*values, value;
  WlzPlaneDomain 	*planedm;
  WlzVoxelValues 	*voxtab;
  WlzPixelV		bgd;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF ){
    return WLZ_ERR_READ_INCOMPLETE;
  }
  if( type == WLZ_NULL ){
    obj->values.core = NULL;
    return WLZ_ERR_NONE;
  }
  if( obj->domain.core == NULL ){
    return WLZ_ERR_DOMAIN_NULL;
  }
    
  planedm = obj->domain.p;
  domains = planedm->domains;
  nplanes = planedm->lastpl - planedm->plane1 + 1;

  /* the background value is ill-used, currently the type is not
     written to disc therefore int assumed. The grey-types of the
     value tables are assumed to be the same therefore we reset the
     voxeltable background type and value to be that of any of the
     valuetable background values - should be changed when the file
     format is changed */
  bgd.type = WLZ_GREY_INT;
  bgd.v.inv = 0;

  switch( type ){

  case WLZ_VOXELVALUETABLE_GREY:
    if( voxtab = WlzMakeVoxelValueTb(WLZ_VOXELVALUETABLE_GREY,
				     planedm->plane1, planedm->lastpl,
				     bgd, obj, &errNum) ){
      values = voxtab->values;
      voxtab->bckgrnd.v.inv = getword(fp);
    }
    else {
      return errNum;
    }

    for(i=0; i < nplanes; i++, values++, domains++){
      (*values).core = NULL;
      if( tmpobj = WlzMakeMain(WLZ_2D_DOMAINOBJ, *domains, *values,
			       NULL, NULL, &errNum) ){
	if( (errNum = WlzReadGreyValues(fp, tmpobj)) == WLZ_ERR_NONE ){
	  *values = WlzAssignValues(tmpobj->values, NULL);
	  /* reset voxel-table background */
	  if( (*values).core != NULL ){
	    switch( WlzGreyTableTypeToTableType((*values).core->type, NULL) ){
	    case WLZ_GREY_TAB_RAGR:
	      voxtab->bckgrnd = (*values).v->bckgrnd;
	      break;
	    case WLZ_GREY_TAB_RECT:
	      voxtab->bckgrnd = (*values).r->bckgrnd;
	      break;
	    case WLZ_GREY_TAB_INTL:
	      voxtab->bckgrnd = (*values).i->bckgrnd;
	      break;
	    }
	  }
	}
	else {
	  (*values).core = NULL;
	  WlzFreeDomain(*domains);
	  (*domains).core = NULL;
	}
	WlzFreeObj( tmpobj );
      }
    }
    break;

  default:
    return WLZ_ERR_VOXELVALUES_TYPE;
    
  }

  if( feof(fp) != 0 ){
    /* allow incomplete object - set domains of unread
       valuetables to empty */
/*    WlzFreeVoxelValueTb( voxtab );*/
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  value.vox = voxtab;
  obj->values = WlzAssignValues(value, NULL);

  return errNum;
}

/************************************************************************
*   Function   : WlzReadPropertyList					*
*   Synopsis   : reads a woolz property list 				*
*   Returns    : WlzSimpleProperty *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: char *err_str:	static string buffer for error message	*
************************************************************************/

static WlzSimpleProperty *WlzReadPropertyList(FILE *fp,
					      WlzErrorNum *dstErr)
{
  WlzObjectType		type;
  WlzSimpleProperty	*pl=NULL;
  int 			si;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = getc(fp);

  switch( type ){

  case (WlzObjectType) EOF:
    errNum = WLZ_ERR_READ_INCOMPLETE;
    break;

  case WLZ_NULL:
    errNum = WLZ_ERR_EOO;
    break;

  default:
    errNum = WLZ_ERR_PROPERTY_TYPE;
    break;

  case WLZ_PROPERTY_SIMPLE:
    /* for compatibility the size read includes an extra sizeof(int)
       which is not actually written to disc */
    si = getword(fp) - sizeof(int);
    if( feof(fp) != 0 || si < 0 ){
      errNum = WLZ_ERR_READ_INCOMPLETE;
      break;
    }

    /* create property list with space for the data */
    if( (pl = WlzMakeSimpleProperty(si, &errNum)) == NULL ){
      break;
    }

    /* The size is now correct for the amount of data */
    if( si > 0 ){  
      fread(pl->prop, si, 1, fp);
    }
    if( feof(fp) != 0 ){
      WlzFreeProperty( pl );
      pl = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    break;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return pl;
}

/************************************************************************
*   Function   : WlzReadPolygon						*
*   Synopsis   : reads a woolz polygon from the given input stream	*
*   Returns    : WlzPolygonDomain *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzPolygonDomain *WlzReadPolygon(FILE *fp,
					WlzErrorNum *dstErr)
{
  WlzObjectType		type;
  WlzPolygonDomain	*poly=NULL;
  WlzFVertex2		*fvtx;
  WlzDVertex2		*dvtx;
  int 			nvertices, i;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  if( type == WLZ_NULL ){
    errNum = WLZ_ERR_EOO;
  }

  nvertices = getword(fp);
  if( feof(fp) != 0 ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }

  if((errNum == WLZ_ERR_NONE) &&
     (poly = WlzMakePolyDmn(type, NULL, 0, nvertices, 1, &errNum)) ){
    poly->nvertices = nvertices;
    switch (type) {

    case WLZ_POLYGON_INT:
      for(i=0; i < nvertices; i++){
	((poly->vtx)+i)->vtY = getword(fp);
	((poly->vtx)+i)->vtX = getword(fp);
      }
      break;

    case WLZ_POLYGON_FLOAT:
      fvtx = (WlzFVertex2 *) poly->vtx;
      for(i=0; i < nvertices; i++){
	((fvtx)+i)->vtY = getfloat(fp);
	((fvtx)+i)->vtX = getfloat(fp);
      }
      break;

    case WLZ_POLYGON_DOUBLE:
      dvtx = (WlzDVertex2 *) poly->vtx;
      for(i=0; i < nvertices; i++){
	((dvtx)+i)->vtY = getdouble(fp);
	((dvtx)+i)->vtX = getdouble(fp);
      }
      break;
    }

    if( feof(fp) != 0 ){
      WlzFreePolyDmn( poly );
      poly = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(poly);
}

/************************************************************************
*   Function   : WlzReadBoundList					*
*   Synopsis   : reads a woolz boundlist  from the given input stream	*
*   Returns    : WlzBoundList *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzBoundList *WlzReadBoundList(FILE *fp,
				      WlzErrorNum *dstErr)
{
  WlzObjectType	type;
  WlzBoundList	*blist=NULL, *tmpblist;
  WlzPolygonDomain	*tmppoly;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);

  switch( type ){

  case (WlzObjectType) EOF:
    errNum = WLZ_ERR_READ_INCOMPLETE;
    break;

  case WLZ_NULL:
    errNum = WLZ_ERR_EOO;
    break;

    /* dummy type written by WriteBoundList, real type read next */
  case (WlzObjectType) 1: 
    type = (WlzObjectType) getc(fp);
    if( (blist = WlzMakeBoundList(type, 0, NULL, &errNum)) == NULL ){
      break;
    }

    if( tmpblist = WlzReadBoundList(fp, &errNum) ){
      blist->next = WlzAssignBoundList(tmpblist, NULL);
    }
    else if( errNum == WLZ_ERR_EOO){
      blist->next = NULL;
      errNum = WLZ_ERR_NONE;
    }
    else {
      WlzFreeBoundList(blist);
      blist = NULL;
      break;
    }

    if( tmpblist = WlzReadBoundList(fp, &errNum) ){
      blist->down = WlzAssignBoundList(tmpblist, NULL);
    }
    else if( errNum == WLZ_ERR_EOO){
      blist->down = NULL;
      errNum = WLZ_ERR_NONE;
    }
    else {
      WlzFreeBoundList(blist);
      blist = NULL;
      break;
    }

    blist->wrap = getword(fp);
    if( tmppoly = WlzReadPolygon(fp, &errNum) ){
      blist->poly = WlzAssignPolygonDomain(tmppoly, NULL);
    }
    else if( errNum == WLZ_ERR_EOO){
      blist->poly = NULL;
      errNum = WLZ_ERR_NONE;
    }
    else{
      WlzFreeBoundList(blist);
      blist = NULL;
      break;
    }

    if( feof(fp) != 0 ){
      WlzFreeBoundList(blist);
      blist = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    break;

  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return( blist );
}

/************************************************************************
*   Function   : WlzReadRect						*
*   Synopsis   : reads a woolz rectangle  from the given input stream	*
*   Returns    : WlzIRect *:						*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: char *err_str:	static string buffer for error message	*
************************************************************************/

static WlzIRect *WlzReadRect(FILE *fp,
			    WlzErrorNum *dstErr)
{
  WlzObjectType	type;
  WlzIRect 	*ir=NULL;
  WlzFRect 	*fr;
  int		i;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  switch( type ){

  case (WlzObjectType) EOF:
    errNum = WLZ_ERR_READ_INCOMPLETE;
    break;

  case WLZ_NULL:
    errNum = WLZ_ERR_EOO;
    break;

  case WLZ_RECTANGLE_DOMAIN_INT:
    if( (ir = (WlzIRect *) AlcMalloc (sizeof(WlzIRect))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    ir->type = type;
    ir->linkcount = 0;
    ir->freeptr = NULL;
    for(i=0; i < 4; i++){
      ir->irk[i] = getword(fp);
    }
    for(i=0; i < 4; i++){
      ir->irl[i] = getword(fp);
    }
    ir->rangle = getfloat(fp);
    break;

  case WLZ_RECTANGLE_DOMAIN_FLOAT:
    if( (fr = (WlzFRect *) AlcMalloc (sizeof(WlzFRect))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    fr->type = type;
    fr->linkcount = 0;
    fr->freeptr = NULL;
    for(i=0; i < 4; i++){
      fr->frk[i] = getfloat(fp);
    }
    for(i=0; i < 4; i++){
      fr->frl[i] = getfloat(fp);
    }
    fr->rangle = getfloat(fp);
    ir = (WlzIRect *) fr;
    break;

  default:
    errNum = WLZ_ERR_OBJECT_TYPE;
    break;
  }

  if( (errNum == WLZ_ERR_NONE) && (feof(fp) != 0) ){
    AlcFree( (void *) ir );
    ir = NULL;
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return( ir );
}

/************************************************************************
*   Function   : WlzReadVector						*
*   Synopsis   : reads a woolz vector from the given input stream	*
*   Returns    : WlzObject *:						*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObjectType type: object type read by WlzReadObj	*
*   Global refs: -							*
************************************************************************/

static WlzObject *WlzReadVector(FILE		*fp,
				WlzObjectType	type,
				WlzErrorNum	*dstErr)
{
  WlzIVector 	*iv=NULL;
  WlzFVector 	*fv;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  switch( type ){

  case WLZ_VECTOR_INT:
    if( (iv = (WlzIVector *) AlcMalloc (sizeof(WlzIVector))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    iv->type = type;
    iv->k1 = getword(fp);
    iv->l1 = getword(fp);
    iv->k2 = getword(fp);
    iv->l2 = getword(fp);
    iv->style = getword(fp);
    if( feof(fp) != 0 ){
      AlcFree( (void *) iv );
      iv = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    break;

  case WLZ_VECTOR_FLOAT:
    if( (fv = (WlzFVector *) AlcMalloc (sizeof(WlzFVector))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    fv->type = type;
    fv->k1 = getfloat(fp);
    fv->l1 = getfloat(fp);
    fv->k2 = getfloat(fp);
    fv->l2 = getfloat(fp);
    fv->style = getword(fp);
    if( feof(fp) != 0 ){
      AlcFree( (void *) fv );
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    break;

  default:
    /* should never get here because the type is checked
       by WlzReadObj */
    errNum = WLZ_ERR_OBJECT_TYPE;
    break;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return (WlzObject *) iv;
}

/************************************************************************
*   Function   : WlzReadPoint						*
*   Synopsis   : reads a woolz point from the given input stream	*
*   Returns    : WlzObject *:						*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObjectType type: object type read by WlzReadObj	*
*   Global refs: -							*
************************************************************************/

static WlzObject *WlzReadPoint(FILE		*fp,
			       WlzObjectType	type,
			       WlzErrorNum	*dstErr)
{
  WlzIPoint 	*iv=NULL;
  WlzFPoint 	*fv;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  switch( type ){

  case WLZ_POINT_INT:
    if( (iv = (WlzIPoint *) AlcMalloc (sizeof(WlzIPoint))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    iv->type = type;
    iv->linkcount = 0;
    iv->k = getword(fp);
    iv->l = getword(fp);
    iv->style = getword(fp);
    if( feof(fp) != 0 ){
      AlcFree( (void *) iv );
      iv = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
      break;
    }
    break;

  case WLZ_POINT_FLOAT:
    if( (fv = (WlzFPoint *) AlcMalloc (sizeof(WlzFPoint))) == NULL){
      errNum = WLZ_ERR_MEM_ALLOC;
      break;
    }
    fv->type = type;
    fv->linkcount = 0;
    fv->k = getfloat(fp);
    fv->l = getfloat(fp);
    fv->style = getword(fp);
    if( feof(fp) != 0 ){
      AlcFree( (void *) fv );
      errNum = WLZ_ERR_READ_INCOMPLETE;
      break;
    }
    iv = (WlzIPoint *) fv;
    break;

  default:
    /* should never get here because the type is checked
       by WlzReadObj */
    errNum = WLZ_ERR_OBJECT_TYPE;
    break;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return (WlzObject *) iv;
}

/************************************************************************
*   Function   : WlzReadHistogramDomain					*
*   Synopsis   : reads a woolz histogram domain				*
*   Returns    : WlzHistogramDomain *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/
static WlzHistogramDomain *WlzReadHistogramDomain(FILE *fp,
						  WlzErrorNum *dstErr)
{
  int		tI0,
  		numBins;
  double	origin,
  		binSize;
  int		*tIP0;
  double	*tDP0;
  WlzObjectType	type,
  		newType;
  WlzHistogramDomain *hist = NULL;
  WlzErrorNum	errNum = WLZ_ERR_NONE;

  type = (WlzObjectType )getc(fp);
  if(type == (WlzObjectType) EOF )
  {
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == WLZ_NULL )
  {
    errNum = WLZ_ERR_EOO;
  }
  else
  {
    switch(type)
    {
      case WLZ_HISTOGRAMDOMAIN_OLD_INT:
      case WLZ_HISTOGRAMDOMAIN_OLD_FLOAT:
        (void )getword(fp);
	(void )getword(fp);
	(void )getword(fp);
	numBins = getword(fp);
	newType = (type == WLZ_HISTOGRAMDOMAIN_OLD_INT)?
		  WLZ_HISTOGRAMDOMAIN_INT:
		  WLZ_HISTOGRAMDOMAIN_FLOAT;
	if(feof(fp))
	{
	  errNum = WLZ_ERR_READ_INCOMPLETE;
	}
	else if((hist = WlzMakeHistogramDomain(newType, numBins,
					       NULL)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  hist->nBins = numBins;
	  hist->origin = 0.0;
	  hist->binSize = 1.0;
	  switch(type)
	  {
	    case WLZ_HISTOGRAMDOMAIN_OLD_INT:
	      tI0 = numBins;
	      tIP0 = hist->binValues.inp;
	      while(tI0-- > 0)
	      {
		*tIP0++ = getword(fp);
	      }
	      break;
	    case WLZ_HISTOGRAMDOMAIN_OLD_FLOAT:
	      tI0 = numBins;
	      tDP0 = hist->binValues.dbp;
	      while(tI0-- > 0)
	      {
		*tDP0++ = getfloat(fp);
	      }
	      break;
	  }
	}
	break;
      case WLZ_HISTOGRAMDOMAIN_INT:
      case WLZ_HISTOGRAMDOMAIN_FLOAT:
	numBins = getword(fp);
	origin = getdouble(fp);
	binSize = getdouble(fp);
	if(feof(fp))
	{
	  errNum = WLZ_ERR_READ_INCOMPLETE;
	}
	else if((hist = WlzMakeHistogramDomain(type, numBins,
					       NULL)) == NULL)
	{
	  errNum = WLZ_ERR_MEM_ALLOC;
	}
	else
	{
	  switch(type)
	  {
	    case WLZ_HISTOGRAMDOMAIN_INT:
	      tI0 = numBins;
	      tIP0 = hist->binValues.inp;
	      while(tI0-- > 0)
	      {
		*tIP0++ = getword(fp);
	      }
	      break;
	    case WLZ_HISTOGRAMDOMAIN_FLOAT:
	      tI0 = numBins;
	      tDP0 = hist->binValues.dbp;
	      while(tI0-- > 0)
	      {
		*tDP0++ = getdouble(fp);
	      }
	      break;
	  }
	  hist->nBins = numBins;
	  hist->origin = origin;
	  hist->binSize = binSize;
	}
	break;
      default:
	errNum = WLZ_ERR_OBJECT_TYPE;
        break;
    }
    if((errNum == WLZ_ERR_NONE) && feof(fp))
    {
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    if(errNum != WLZ_ERR_NONE)
    {
      if(hist)
      {
        WlzFreeHistogramDomain(hist);
	hist = NULL;
      }
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(hist);
}

/************************************************************************
*   Function   : WlzReadCompoundA					*
*   Synopsis   : reads a woolz compund object				*
*   Returns    : WlzObject *:						*
*   Parameters : FILE *fp:	input stream				*
*		 WlzObjectType type: object type read by WlzReadObj	*
*   Global refs: -							*
************************************************************************/

static WlzObject *WlzReadCompoundA(FILE			*fp,
				   WlzObjectType	type,
				   WlzErrorNum		*dstErr)
{
  WlzCompoundArray	*c=NULL;
  WlzObjectType		otype;
  int 			i, n;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  otype = (WlzObjectType) getc(fp);
  if( otype == (WlzObjectType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( (type == WLZ_COMPOUND_ARR_1) && (otype == WLZ_NULL) ){
    errNum = WLZ_ERR_EOO;
  }
  else {
    n = getword(fp);
    if( feof(fp) != 0 ){
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
  }

  if((errNum == WLZ_ERR_NONE) &&
     (c = WlzMakeCompoundArray(type, 1, n, NULL, otype, &errNum)) ){
    for(i=0; (i<n) && (errNum == WLZ_ERR_NONE); i++){
      c->o[i] = WlzAssignObject(WlzReadObj(fp, &errNum), NULL);
    }
    if( errNum == WLZ_ERR_NONE ){
      c->p = WlzAssignProperty(WlzReadPropertyList(fp, NULL), NULL);
    }
    else {
      WlzFreeObj((WlzObject *) c);
      c = NULL;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return( (WlzObject *) c );
}
	
/************************************************************************
*   Function   : WlzReadAffineTransform					*
*   Synopsis   : reads a woolz transform from the given input stream	*
*   Returns    : WlzAffineTransform *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzAffineTransform *WlzReadAffineTransform(FILE *fp,
						  WlzErrorNum *dstErr)
{
  WlzTransformType	type;
  WlzAffineTransform	*trans=NULL;
  int			i, j;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  type = (WlzTransformType) getc( fp );
  if( type == (WlzTransformType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == (WlzTransformType) WLZ_NULL ){
    errNum = WLZ_ERR_EOO;
  }
  else if( trans = WlzMakeAffineTransform(type, &errNum) ){

    /* set linkcount and freeptr */
    trans->linkcount = 0;
    trans->freeptr   = NULL;

    /* read parameter values */
    trans->tx = getdouble(fp);
    trans->ty = getdouble(fp);
    trans->tz = getdouble(fp);
    trans->scale = getdouble(fp);
    trans->theta = getdouble(fp);
    trans->phi = getdouble(fp);
    trans->alpha = getdouble(fp);
    trans->psi = getdouble(fp);
    trans->xsi = getdouble(fp);
    trans->invert = getword(fp);

    /* read the matrix */
    for(i=0; i < 4; i++){
      for(j=0; j < 4; j++){
	trans->mat[i][j] = getdouble( fp );
      }
    }

    if( feof(fp) != 0 ){
      WlzFreeAffineTransform( trans );
      trans = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return( trans );
}

/************************************************************************
*   Function   : WlzReadWarpTrans					*
*   Synopsis   : reads a woolz warp transform				*
*   Returns    : WlzWarpTrans *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzWarpTrans *WlzReadWarpTrans(FILE *fp,
				      WlzErrorNum *dstErr)
{
  /* local variables */
  int	       	i, j;
  WlzObjectType type;
  WlzDVertex2 	*dptr;
  WlzTElement 	*eptr;
  WlzWarpTrans 	*obj=NULL;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  type = (WlzObjectType) getc(fp);
  if( type == (WlzObjectType) EOF ){
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }
  else if( type == WLZ_NULL ){
    errNum = WLZ_ERR_EOO;
  }
  /* this is a repeat of the same char read by WlzReadObj 
     and therefore is redundant, this can only be different if the file
     is corrupt */
  else if( type != WLZ_WARP_TRANS ){
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  /* make space for obj */
  else if( (obj = (WlzWarpTrans *) AlcMalloc(sizeof(WlzWarpTrans)))
	  == NULL ){
    errNum = WLZ_ERR_MEM_ALLOC;
  }
  else {
    obj->type = type;
    obj->linkcount = 0;

    obj->nelts = getword(fp);
    obj->nodes = getword(fp);
    obj->imdisp = getfloat(fp);
    obj->iterdisp = getfloat(fp);
    if( feof(fp) != 0 ){
      AlcFree( (void *) obj );
      obj = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
  }

  /* read nodal coords */
  if( errNum == WLZ_ERR_NONE ){
    if( (obj->ncoords = (WlzDVertex2 *)
	 AlcMalloc(sizeof(WlzDVertex2) * obj->nodes)) == NULL ){
      AlcFree( (void *) obj );
      obj = NULL;
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else {
      dptr = obj->ncoords;
      for(i=0; i<obj->nodes; i++, dptr++){
	dptr->vtX = (double) getfloat(fp);
	dptr->vtY = (double) getfloat(fp);
      }
    }
  }

  /* read nodal displacements */
  if( errNum == WLZ_ERR_NONE ){
    if( (obj->displacements = (WlzDVertex2 *)
	 AlcMalloc(sizeof(WlzDVertex2) * obj->nodes)) == NULL ){
      AlcFree( (void *) obj->ncoords );
      AlcFree( (void *) obj );
      obj = NULL;
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else {
      dptr = obj->displacements ;
      for(i=0; i<obj->nodes; i++, dptr++){
	dptr->vtX = (double) getfloat(fp);
	dptr->vtY = (double) getfloat(fp);
      }
    }
  }

  /* read elements */
  if( errNum == WLZ_ERR_NONE ){
    if( (obj->eltlist = (WlzTElement *)
	 AlcMalloc(sizeof(WlzTElement) * obj->nelts)) == NULL ){
      AlcFree( (void *) obj->nodes );
      AlcFree( (void *) obj->ncoords );
      AlcFree( (void *) obj );
      obj = NULL;
      errNum = WLZ_ERR_MEM_ALLOC;
    }
    else {
      eptr = obj->eltlist;
      for(i=0; i<obj->nelts; i++, eptr++){
	eptr->type = (int) getc(fp);
	eptr->n = getword(fp);
	for(j=0; j<3; j++){
	  eptr->nodes[j] = getword(fp);
	}
	for (j=0; j<3; j++){
	  eptr->u[j] = getfloat(fp);
	}
	for (j=0; j<3; j++){
	  eptr->a[j] = getfloat(fp);
	}
      }
    }
  }

  /* check if EOF error has been set */
  if( (errNum == WLZ_ERR_NONE) && (feof(fp) != 0) ){
    AlcFree( (void *) obj->eltlist );
    AlcFree( (void *) obj->nodes );
    AlcFree( (void *) obj->ncoords );
    AlcFree((void *) obj);
    obj = NULL;
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj) ;
}

/************************************************************************
*   Function   : WlzReadFMatchObj					*
*   Synopsis   : reads a woolz match object - used for warping		*
*   Returns    : WlzFMatchObj *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/

static WlzFMatchObj *WlzReadFMatchObj(FILE *fp,
				      WlzErrorNum *dstErr)
{
  int 			i, j;
  WlzFMatchPoint	*mptr;
  WlzFMatchObj		*obj=NULL;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  if( (obj = (WlzFMatchObj *) AlcMalloc(sizeof(WlzFMatchObj))) == NULL ){
    errNum = WLZ_ERR_MEM_ALLOC;
  }
  else {
    obj->type = WLZ_FMATCHOBJ;
    obj->linkcount = 0;

    obj->nopts = getword(fp);
    if( feof(fp) != 0 ){
      AlcFree((void *) obj);
      obj = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
    else {
      if( (obj->matchpts = (WlzFMatchPoint *)
	   AlcMalloc(sizeof(WlzFMatchPoint) * obj->nopts)) == NULL ){
	AlcFree((void *) obj);
	obj = NULL;
	errNum = WLZ_ERR_MEM_ALLOC;
      }
      else {
	mptr = obj->matchpts;
	for(i=0; i<obj->nopts; i++, mptr++){
	  mptr->type = getword(fp);
	  mptr->node = getword(fp);
	  mptr->ptcoords.vtX = getfloat(fp);
	  mptr->ptcoords.vtY = getfloat(fp);

	  for(j=0; j<WLZ_MAX_NODAL_DEGREE; j++){
	    mptr->elements[j] = getword(fp);
	  }
	}
      }
    }
  }

  if( (errNum == WLZ_ERR_NONE) && (feof(fp) != 0) ){
    AlcFree((void *) obj->matchpts);
    AlcFree((void *) obj);
    obj = NULL;
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}

/************************************************************************
*   Function   : WlzRead3DWarpTrans					*
*   Synopsis   : reads a woolz  3D warp transform			*
*   Returns    : Wlz3DWarpTrans *:					*
*   Parameters : FILE *fp:	input stream				*
*   Global refs: -							*
************************************************************************/
static Wlz3DWarpTrans *WlzRead3DWarpTrans(FILE *fp,
					  WlzErrorNum *dstErr)
{
  /* local variables */
  int 			i, nplanes;
  Wlz3DWarpTrans	*obj=NULL;
  WlzFMatchObj		**intdoms;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  if( (obj = (Wlz3DWarpTrans *) AlcMalloc(sizeof(Wlz3DWarpTrans)))
     == NULL ){
    errNum = WLZ_ERR_MEM_ALLOC;
  }
  else {
    obj->type = WLZ_3D_WARP_TRANS;
    obj->linkcount = 0;
    obj->iteration = getword(fp);
    obj->currentplane = getword(fp);
    obj->maxdisp = getfloat(fp);
    if( feof(fp) != 0 ){
      AlcFree((void *) obj);
      obj = NULL;
      errNum = WLZ_ERR_READ_INCOMPLETE;
    }
  }

  if( errNum == WLZ_ERR_NONE ){
    if( (obj->pdom = WlzReadPlaneDomain(fp, &errNum)) == NULL ){
      AlcFree((void *) obj);
      obj = NULL;
    }
    else {
      obj->pdom->linkcount = 1;
      nplanes = obj->pdom->lastln - obj->pdom->line1 + 1;
      if( (obj->intptdoms = (WlzFMatchObj **)
	   AlcMalloc(sizeof(WlzFMatchObj *) * nplanes)) == NULL ){
	WlzFreePlaneDomain( obj->pdom );
	AlcFree((void *) obj);
	obj = NULL;
	errNum = WLZ_ERR_MEM_ALLOC;
      }
      else {
	intdoms = obj->intptdoms;
	for(i=obj->pdom->plane1; i<=obj->pdom->lastpl; i++, intdoms++){
	  if( (*intdoms = WlzReadFMatchObj(fp, &errNum)) ){
	    (*intdoms)->linkcount = 1;
	  }
	  else if( errNum != WLZ_ERR_EOO ){
	    WlzFree3DWarpTrans(obj);
	    obj = NULL;
	  }
	}
      }
    }
  }

  if( (errNum == WLZ_ERR_NONE) && (feof(fp) != 0) ){
    WlzFree3DWarpTrans(obj);
    obj = NULL;
    errNum = WLZ_ERR_READ_INCOMPLETE;
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return(obj);
}
