#pragma ident "MRC HGU $Id$"
/*!
* \file         WlzGreySetRangeLut.c
* \author       Richard Baldock <Richard.Baldock@hgu.mrc.ac.uk>
* \date         Thu Nov 15 15:43:55 2001
* \version      MRC HGU $Id$
*               $Revision$
*               $Name$
* \par Copyright:
*               1994-2001 Medical Research Council, UK.
*               All rights reserved.
* \par Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \brief        Function to transform the grey-values of a grey-level
 object using a look-up-table.
*               
* \todo         -
* \bug          None known
*
* Maintenance log with most recent changes at top of list.
*/

#include <stdlib.h>
#include <float.h>
#include <Wlz.h>

WlzErrorNum WlzGreySetRangeLut(
  WlzObject	*obj,
  WlzPixelV	min,
  WlzPixelV	max,
  WlzPixelP	lut)
{
  WlzIntervalWSpace	iwsp;
  WlzGreyWSpace		gwsp;
  WlzGreyP		gptr;
  WlzObject		*tempobj;
  WlzValues 		*values;
  WlzDomain		*domains;
  int			i, nplanes;
  WlzErrorNum		wlzErrno=WLZ_ERR_NONE;
  int			minV;

  /* check object */
  if( obj == NULL ){
    wlzErrno = WLZ_ERR_OBJECT_NULL;
  }

  if( wlzErrno == WLZ_ERR_NONE ){
    switch( obj->type ){

    case WLZ_2D_DOMAINOBJ:
      if( obj->domain.i == NULL ){
	return WLZ_ERR_DOMAIN_NULL;
      }
      if( obj->values.v == NULL ){
	return WLZ_ERR_VALUES_NULL;
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      /* check planedomain and voxeltable */
      if( obj->domain.p == NULL ){
	return WLZ_ERR_DOMAIN_NULL;
      }
      if( obj->domain.p->type != WLZ_PLANEDOMAIN_DOMAIN ){
	return WLZ_ERR_PLANEDOMAIN_TYPE;
      }
      if( obj->values.vox == NULL ){
	return WLZ_ERR_VALUES_NULL;
      }
      if( obj->values.vox->type != WLZ_VOXELVALUETABLE_GREY ){
	return WLZ_ERR_VOXELVALUES_TYPE;
      }

      /* set range of each plane if non-empty - indicated by NULL */
      domains = obj->domain.p->domains;
      values = obj->values.vox->values;
      nplanes = obj->domain.p->lastpl - obj->domain.p->plane1 + 1;
      for(i=0; i < nplanes; i++, domains++, values++){

	if( (*domains).core == NULL || (*values).core == NULL ){
	  continue;
	}

	tempobj = WlzMakeMain(WLZ_2D_DOMAINOBJ,
			      *domains, *values, NULL, NULL,
			      &wlzErrno);
	if((tempobj == NULL) && (wlzErrno == WLZ_ERR_NONE) ){
	  wlzErrno = WLZ_ERR_UNSPECIFIED;
	  break;
	}

	wlzErrno = WlzGreySetRangeLut(tempobj, min, max, lut);
	WlzFreeObj( tempobj );
	if( wlzErrno != WLZ_ERR_NONE ){
	  break;
	}
      }
      
      return wlzErrno;

    case WLZ_TRANS_OBJ:
      return WlzGreySetRangeLut(obj->values.obj, min, max, lut);

    case WLZ_EMPTY_OBJ:
      return wlzErrno;

    default:
      wlzErrno = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( wlzErrno == WLZ_ERR_NONE ){
    WlzValueConvertPixel(&min, min, WLZ_GREY_INT);
    WlzValueConvertPixel(&max, max, WLZ_GREY_INT);
    minV = min.v.inv;

    WlzInitGreyScan(obj, &iwsp, &gwsp);
    while( WlzNextGreyInterval(&iwsp) == WLZ_ERR_NONE ){

      gptr = gwsp.u_grintptr;
      switch (gwsp.pixeltype) {

      case WLZ_GREY_INT:
	for (i=0; i<iwsp.colrmn; i++, gptr.inp++)
	  *gptr.inp = lut.p.ubp[*gptr.inp - minV];
	break;

      case WLZ_GREY_SHORT:
	for (i=0; i<iwsp.colrmn; i++, gptr.shp++)
	  *gptr.shp = lut.p.ubp[*gptr.shp - minV];
	break;

      case WLZ_GREY_UBYTE:
	for (i=0; i<iwsp.colrmn; i++, gptr.ubp++)
	  *gptr.ubp = lut.p.ubp[*gptr.ubp - minV];
	break;

      case WLZ_GREY_FLOAT:
	for (i=0; i<iwsp.colrmn; i++, gptr.flp++)
	  *gptr.flp = lut.p.ubp[(int) *gptr.flp - minV];
	break;

      case WLZ_GREY_DOUBLE:
	for (i=0; i<iwsp.colrmn; i++, gptr.dbp++)
	  *gptr.dbp = lut.p.ubp[(int) *gptr.dbp - minV];
	break;

      default:
	wlzErrno = WLZ_ERR_GREY_TYPE;
	break;
      }
    }
  }

  if( wlzErrno == WLZ_ERR_EOO ){
    wlzErrno = WLZ_ERR_NONE;
  }
  return wlzErrno;
}
