#pragma ident "MRC HGU $Id$"
/*!
* \file         WlzGreyModGradient.c
* \author       richard <Richard.Baldock@hgu.mrc.ac.uk>
* \date         Fri Sep 26 11:52:24 2003
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
* \brief        Functions to calculate the modulus of the grey-level
 gradient of Woolz objects. 
*               
* \todo         -
* \bug          None known
*
* Maintenance log with most recent changes at top of list.
*/

#include <stdlib.h>
#include <Wlz.h>


/* function:     WlzGreyModGradient    */
/*! 
* \ingroup      WlzValuesUtils
* \brief        Calculate the modulus of the grey-level gradient at each
 point. The gradient images are calculated using WlzGauss2() with width
parameter set to <tt>width</tt>.
*
* \return       Object with values set to the gradient modulus at each pixel.
* \param    obj	Input object.
* \param    width	Width parameter for the gaussian gradient operator.
* \param    dstErr	Error return.
* \par      Source:
*                WlzGreyModGradient.c
*/
WlzObject *WlzGreyModGradient(
  WlzObject	*obj,
  double	width,
  WlzErrorNum	*dstErr)
{
  WlzObject		*xobj, *yobj, *returnobj=NULL;
  WlzIntervalWSpace	iwsp1, iwsp2, iwsp3;
  WlzGreyWSpace		gwsp1, gwsp2, gwsp3;
  int			i;
  double		g1, g2, g3;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  /* check object */
  if( obj ){
    switch( obj->type ){
    case WLZ_2D_DOMAINOBJ:
      if( obj->domain.core ){
	switch( obj->domain.core->type ){
	case WLZ_EMPTY_DOMAIN:
	  returnobj = WlzMakeEmpty(&errNum);
	  break;

	default:
	  if( obj->values.core ){
	    if( obj->values.core->type == WLZ_EMPTY_VALUES ){
	      errNum = WLZ_ERR_VALUES_TYPE;
	    }
	  }
	  else {
	    errNum = WLZ_ERR_VALUES_NULL;
	  }
	  break;
	}
      }
      else {
	errNum = WLZ_ERR_DOMAIN_NULL;
      }
      break;

    case WLZ_EMPTY_OBJ:
      returnobj = WlzMakeEmpty(&errNum);
      break;

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  /* if UBYTE grey values then copy to int */
  if( (errNum == WLZ_ERR_NONE) && !returnobj ){
    if(WlzGreyTableTypeToGreyType(obj->values.core->type, NULL) == WLZ_GREY_UBYTE)
    {
      returnobj = WlzConvertPix(obj, WLZ_GREY_INT, NULL);
    }
    else
    {
      returnobj = WlzMakeMain(WLZ_2D_DOMAINOBJ, obj->domain, obj->values,
			      NULL, NULL, NULL);
    }

    /* calculate gradient images */
    xobj = WlzGauss2(returnobj, width, width, 1, 0, NULL);
    yobj = WlzGauss2(returnobj, width, width, 0, 1, NULL);

    /* calculate modulus  - lockstep raster scan assumes equal domains */
    errNum = WlzInitGreyScan(returnobj, &iwsp1, &gwsp1);
    errNum = WlzInitGreyScan(xobj, &iwsp2, &gwsp2);
    errNum = WlzInitGreyScan(yobj, &iwsp3, &gwsp3);
    while((errNum == WLZ_ERR_NONE) && 
	  (WlzNextGreyInterval(&iwsp1) == WLZ_ERR_NONE) )
    {
      (void) WlzNextGreyInterval(&iwsp2);
      (void) WlzNextGreyInterval(&iwsp3);

      switch( gwsp1.pixeltype )
      {
      default:
      case WLZ_GREY_INT:
	for(i=0; i < iwsp1.colrmn; i++)
	{
	  g2 = *gwsp2.u_grintptr.inp++;
	  g3 = *gwsp3.u_grintptr.inp++;
	  g1 = sqrt( g2*g2 + g3*g3 );
	  *gwsp1.u_grintptr.inp++ = (int) g1;
	}
	break;

      case WLZ_GREY_SHORT:
	for(i=0; i < iwsp1.colrmn; i++)
	{
	  g2 = *gwsp2.u_grintptr.shp++;
	  g3 = *gwsp3.u_grintptr.shp++;
	  g1 = sqrt( g2*g2 + g3*g3 );
	  *gwsp1.u_grintptr.shp++ = (short) g1;
	}
	break;

      case WLZ_GREY_UBYTE:
	for(i=0; i < iwsp1.colrmn; i++)
	{
	  g2 = *gwsp2.u_grintptr.ubp++;
	  g3 = *gwsp3.u_grintptr.ubp++;
	  g1 = sqrt( g2*g2 + g3*g3 );
	  *gwsp1.u_grintptr.ubp++ = (UBYTE) g1;
	}
	break;

      case WLZ_GREY_FLOAT:
	for(i=0; i < iwsp1.colrmn; i++)
	{
	  g2 = *gwsp2.u_grintptr.flp++;
	  g3 = *gwsp3.u_grintptr.flp++;
	  g1 = sqrt( g2*g2 + g3*g3 );
	  *gwsp1.u_grintptr.flp++ = (float) g1;
	}
	break;

      case WLZ_GREY_DOUBLE:
	for(i=0; i < iwsp1.colrmn; i++)
	{
	  g2 = *gwsp2.u_grintptr.dbp++;
	  g3 = *gwsp3.u_grintptr.dbp++;
	  g1 = sqrt( g2*g2 + g3*g3 );
	  *gwsp1.u_grintptr.dbp++ = (double) g1;
	}
	break;

      case WLZ_GREY_RGBA: /* RGBA to be done - not sure what RAB */
	errNum = WLZ_ERR_GREY_TYPE;
	break;

      }
    }

    /* clean up */
    WlzFreeObj( xobj );
    WlzFreeObj( yobj );
  }

  /* check error return */
  if( dstErr ){
    *dstErr = errNum;
  }
  return( returnobj );
}
