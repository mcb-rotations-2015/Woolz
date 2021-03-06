#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzGreySetRangeLut_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzGreySetRangeLut.c
* \author       Richard Baldock
* \date         November 2001
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Transforms the grey-values of a grey-level object using a
* 		look-up-table.
* \ingroup	WlzValuesFilters
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
  WlzErrorNum		errNum=WLZ_ERR_NONE;
  int			minV;

  /* check object */
  if( obj == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }

  if( errNum == WLZ_ERR_NONE ){
    switch( obj->type ){

    case WLZ_2D_DOMAINOBJ:
      if( obj->domain.i == NULL ){
	return WLZ_ERR_DOMAIN_NULL;
      }
      if( obj->values.core == NULL ){
	return WLZ_ERR_VALUES_NULL;
      }
      if( WlzGreyTableIsTiled(obj->values.core->type) ) {
        return WLZ_ERR_VALUES_TYPE;
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
			      &errNum);
	if((tempobj == NULL) && (errNum == WLZ_ERR_NONE) ){
	  errNum = WLZ_ERR_UNSPECIFIED;
	  break;
	}

	errNum = WlzGreySetRangeLut(tempobj, min, max, lut);
	WlzFreeObj( tempobj );
	if( errNum != WLZ_ERR_NONE ){
	  break;
	}
      }
      
      return errNum;

    case WLZ_TRANS_OBJ:
      return WlzGreySetRangeLut(obj->values.obj, min, max, lut);

    case WLZ_EMPTY_OBJ:
      return errNum;

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( errNum == WLZ_ERR_NONE ){
    WlzValueConvertPixel(&min, min, WLZ_GREY_INT);
    WlzValueConvertPixel(&max, max, WLZ_GREY_INT);
    minV = min.v.inv;

    errNum = WlzInitGreyScan(obj, &iwsp, &gwsp);
    if(errNum == WLZ_ERR_NONE) {
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

	case WLZ_GREY_RGBA:
	  for (i=0; i<iwsp.colrmn; i++, gptr.rgbp++){
	    WlzUInt red, green, blue, alpha;
	    red = WLZ_RGBA_RED_GET(*gptr.rgbp);
	    green = WLZ_RGBA_GREEN_GET(*gptr.rgbp);
	    blue = WLZ_RGBA_BLUE_GET(*gptr.rgbp);
	    alpha = WLZ_RGBA_ALPHA_GET(*gptr.rgbp);
	    red = lut.p.ubp[red - minV];
	    green = lut.p.ubp[green - minV];
	    blue = lut.p.ubp[blue - minV];
	    WLZ_RGBA_RED_SET(*gptr.rgbp, red);
	    WLZ_RGBA_GREEN_SET(*gptr.rgbp, green);
	    WLZ_RGBA_BLUE_SET(*gptr.rgbp, blue);
	    WLZ_RGBA_ALPHA_SET(*gptr.rgbp, alpha);
	  }
	  break;

	default:
	  errNum = WLZ_ERR_GREY_TYPE;
	  break;
	}
      }
      (void )WlzEndGreyScan(&iwsp, &gwsp);
    }
  }

  if( errNum == WLZ_ERR_EOO ){
    errNum = WLZ_ERR_NONE;
  }
  return errNum;
}
