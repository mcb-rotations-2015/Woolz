#pragma ident "MRC HGU $Id$"
/*!
* \file         WlzRGBAModGradient.c
* \author       richard <Richard.Baldock@hgu.mrc.ac.uk>
* \date         Mon Jun 20 06:46:50 2005
* \version      MRC HGU $Id$
*               $Revision$
*               $Name$
* \par Copyright:
*               1994-2002 Medical Research Council, UK.
*               All rights reserved.
* \par Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \ingroup      WlzValuesUtils
* \brief        Calculate the modulus of the gradient of a Woolz RGBA
 object. The gradient is defined as the modulus of the "modulus" vector,
 i.e. the modulus for each colour.
*               
* \todo         -
* \bug          None known
*
* Maintenance log with most recent changes at top of list.
*/

#include <stdlib.h>
#include <Wlz.h>


/* function:     WlzRGBAModGradient    */
/*! 
* \ingroup      WlzValuesUtils
* \brief        Calculate the modulus of the rgb-level gradient at each
 point. The gradient images are calculated using WlzGauss2() with width
parameter set to <tt>width</tt>. Note the modulus returned is the modulus
of vector formed by the independent channels, i.e. take the modulus for
each channel seperately then the modulus of the resultant vector.
*
* \return       Woolz object
* \param    obj	Input rgba woolz object
* \param    width	width of gradient operator
* \param    dstErr	error return
* \par      Source:
*                WlzRGBAModGradient.c
*/
extern WlzObject *WlzRGBAModGradient(
  WlzObject	*obj,
  double	width,
  WlzErrorNum	*dstErr)
{
  WlzObject	*rtnObj=NULL;
  WlzCompoundArray	*cobj1=NULL, *cobj2=NULL;
  WlzIntervalWSpace	iwsp[7];
  WlzGreyWSpace		gwsp[7];
  int		i, j;
  double	g1, g2;
  WlzValues	values;
  WlzPixelV	bckgrnd;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* check inputs */
  if( obj == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else {
    switch( obj->type ){
    case WLZ_2D_DOMAINOBJ:
      if(WlzGreyTableTypeToGreyType(obj->values.core->type, NULL)
	 != WLZ_GREY_RGBA){
	return WlzGreyModGradient(obj, width, dstErr);
      }
      break;

    default:
      return WlzGreyModGradient(obj, width, dstErr);
    }
  }

  /* calculate gradient images - now returned as compound objects */
  if((errNum == WLZ_ERR_NONE) && 
     (cobj1 = (WlzCompoundArray *) WlzGauss2(obj, width, width,
					     1, 0, &errNum))){
    if( cobj1->type != WLZ_COMPOUND_ARR_1 ){
      errNum = WLZ_ERR_OBJECT_DATA;
    }
  }
  if((errNum == WLZ_ERR_NONE) && 
     (cobj2 = (WlzCompoundArray *) WlzGauss2(obj, width, width,
					     0, 1, &errNum))){
    if( cobj2->type != WLZ_COMPOUND_ARR_1 ){
      errNum = WLZ_ERR_OBJECT_DATA;
    }
  }
     
  /* returnobj must be at least grey-type short */
  if( errNum == WLZ_ERR_NONE ){
    bckgrnd.type = WLZ_GREY_SHORT;
    bckgrnd.v.shv = 0.0;
    if( values.v = WlzNewValueTb(obj,
				 WlzGreyTableType(WLZ_GREY_TAB_RAGR,
						  WLZ_GREY_SHORT, NULL),
				 bckgrnd, &errNum) ){
      rtnObj = WlzMakeMain(obj->type, obj->domain, values,
			   NULL, NULL, &errNum);
    }
    else {
      WlzFreeObj((WlzObject *) cobj2);
      WlzFreeObj((WlzObject *) cobj1);
    }
  }
			 
  /* now scan lock-step fashion since all domains are identical.
     Note the returnobj has short grey-type so we need only switch
     on the returned type from WlzGauss2. This will be int for now
     but put in check for safetly */
  if( errNum == WLZ_ERR_NONE ){
    errNum = WlzInitGreyScan(rtnObj, &iwsp[0], &gwsp[0]);
    for(i=0; (i < 3) && (errNum == WLZ_ERR_NONE); i++){
      errNum = WlzInitGreyScan(cobj1->o[i], &iwsp[i+1], &gwsp[i+1]);
    }
    for(i=0; (i < 3) && (errNum == WLZ_ERR_NONE); i++){
      errNum = WlzInitGreyScan(cobj2->o[i], &iwsp[i+4], &gwsp[i+4]);
    }

    while((errNum == WLZ_ERR_NONE) && 
	  (WlzNextGreyInterval(&iwsp[0]) == WLZ_ERR_NONE)){
      for(i=1; (i < 7) && (errNum == WLZ_ERR_NONE); i++){
	errNum = WlzNextGreyInterval(&iwsp[i]);
      }

      switch( gwsp[1].pixeltype ){
      case WLZ_GREY_INT:
	for(i=0; i < iwsp[0].colrmn; i++){
	  for(g1=0, j=1; j < 7; j++){
	    g2 = *gwsp[j].u_grintptr.inp++;
	    g1 += g2*g2;
	  }
	  *gwsp[0].u_grintptr.shp++ = (int) sqrt(g1);
	}
	break;

      case WLZ_GREY_SHORT:
	for(i=0; i < iwsp[0].colrmn; i++){
	  for(g1=0, j =1; j < 7; j++){
	    g2 = *gwsp[j].u_grintptr.shp++;
	    g1 += g2*g2;
	  }
	  *gwsp[0].u_grintptr.shp++ = (int) sqrt(g1);
	}
	break;

      case WLZ_GREY_UBYTE:
	for(i=0; i < iwsp[0].colrmn; i++){
	  for(g1=0, j =1; j < 7; j++){
	    g2 = *gwsp[j].u_grintptr.ubp++;
	    g1 += g2*g2;
	  }
	  *gwsp[0].u_grintptr.shp++ = (int) sqrt(g1);
	}
	break;

      case WLZ_GREY_FLOAT:
	for(i=0; i < iwsp[0].colrmn; i++){
	  for(g1=0, j =1; j < 7; j++){
	    g2 = *gwsp[j].u_grintptr.flp++;
	    g1 += g2*g2;
	  }
	  *gwsp[0].u_grintptr.shp++ = (int) sqrt(g1);
	}
	break;

      case WLZ_GREY_DOUBLE:
	for(i=0; i < iwsp[0].colrmn; i++){
	  for(g1=0, j =1; j < 7; j++){
	    g2 = *gwsp[j].u_grintptr.dbp++;
	    g1 += g2*g2;
	  }
	  *gwsp[0].u_grintptr.shp++ = (int) sqrt(g1);
	}
	break;

      default:
	errNum = WLZ_ERR_GREY_TYPE;
	break;
      }
    }

    /* check for normal return */
    if( errNum == WLZ_ERR_EOO ){
      errNum = WLZ_ERR_NONE;
    }
    if( errNum != WLZ_ERR_NONE ){
      WlzFreeObj(rtnObj);
      rtnObj = NULL;
    }
  }

  /* clean up and check error return */
  if( cobj1 ){
    WlzFreeObj((WlzObject *) cobj1);
  }
  if( cobj2 ){
    WlzFreeObj((WlzObject *) cobj2);
  }
  if( dstErr ){
    *dstErr = errNum;
  }
  return rtnObj;
}
