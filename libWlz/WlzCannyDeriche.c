#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzCannyDeriche_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         libWlz/WlzCannyDeriche.c
* \author       Bill Hill
* \date         May 1999
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
* \brief	A Canny-Deriche edge detection filter.
* \ingroup	WlzValuesFilters
*/

#include <stdio.h>
#include <float.h>
#include <string.h>
#include <Wlz.h>

/*!
* \return	Edge detected object or NULL on error.
* \ingroup	WlzValuesFilters
* \brief        A Canny-Deriche edge detection filter which performs
*               the following steps:
*		<ol>
		<li>
		Recursive filter with the Deriche edge operator\n
	        applied in each dimension.
		<li>
		Modulus of gradient image generated.
		<li>
		Non-maximal suppression of the gradient image.
		<li>
		Hysteresis threshold of NM suppressed gradient\n
		image.
		</ol>
* \param	dstGObj			Destination pointer for the gradient
* 					image, may be NULL.
* \param	srcObj			Given domain object with grey values.
* \param	alpha			Deriche filter parameter.
* \param	mult			Filter multiplier.
* \param	pMinGrdV		Primary hysteresis threshold value.
* \param	sMinGrdV		Secondary hysteresis threshold value.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzObject 	*WlzCannyDeriche(WlzObject **dstGObj, WlzObject *srcObj,
				 double alpha, double mult,
				 WlzPixelV pMinGrdV, WlzPixelV sMinGrdV,
			   	 WlzErrorNum *dstErr)
{
  WlzObject     *gMObj = NULL,
		*gZObj = NULL,
		*gYObj = NULL,
		*gXObj = NULL,
		*nMObj = NULL,
		*nMGObj = NULL,
		*hTObj = NULL,
		*dstObj = NULL;
  WlzRsvFilter  *ftr = NULL;
  WlzErrorNum   errNum = WLZ_ERR_NONE;

  if(srcObj == NULL)
  {
    errNum = WLZ_ERR_OBJECT_NULL;
  }
  else if(srcObj->type != WLZ_2D_DOMAINOBJ)
  {
    errNum = WLZ_ERR_OBJECT_TYPE;
  }
  else if(srcObj->domain.core == NULL)
  {
    errNum = WLZ_ERR_DOMAIN_NULL;
  }
  else if(srcObj->values.core == NULL)
  {
    errNum = WLZ_ERR_VALUES_NULL;
  }
  else if((ftr = WlzRsvFilterMakeFilter(WLZ_RSVFILTER_NAME_DERICHE_1,
					alpha, &errNum)) != NULL)
  {
    ftr->c *= mult;
    gMObj = WlzAssignObject(
	    WlzGreyGradient(&gZObj, &gYObj, &gXObj, srcObj, ftr,
			    &errNum), NULL);
    WlzRsvFilterFreeFilter(ftr);
    if(errNum == WLZ_ERR_NONE)
    {
      nMObj = WlzAssignObject(
	      WlzNMSuppress(gMObj, gZObj, gYObj, gXObj, sMinGrdV,
	       		     &errNum), NULL);
    }
    if(gZObj)
    {
      WlzFreeObj(gZObj);
    }
    if(gYObj)
    {
      WlzFreeObj(gYObj);
    }
    if(gXObj)
    {
      WlzFreeObj(gXObj);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      nMGObj = WlzAssignObject(
      	       WlzMakeMain(nMObj->type, nMObj->domain, gMObj->values,
	     		   NULL, gMObj, &errNum), NULL);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      hTObj = WlzAssignObject(
      	      WlzHyThreshold(nMGObj, pMinGrdV, sMinGrdV, WLZ_THRESH_HIGH,
	      		     WLZ_8_CONNECTED, &errNum), NULL);
    }
    if(nMGObj)
    {
      WlzFreeObj(nMGObj);
    }
    if(errNum == WLZ_ERR_NONE)
    {
      dstObj = WlzAssignObject(
      	       WlzMakeMain(hTObj->type, hTObj->domain, nMObj->values,
	     		 NULL, nMObj, &errNum), NULL);
    }
    if(hTObj)
    {
      WlzFreeObj(hTObj);
    }
    if(dstGObj)
    {
      *dstGObj = gMObj;
      gMObj = NULL;
    }
  }
  if(gMObj)
  {
    WlzFreeObj(gMObj);
  }
  if(errNum != WLZ_ERR_NONE)
  {
    if(dstErr)
    {
      *dstErr = errNum;
    }
    dstObj = NULL;
  }
  return(dstObj);
}

/* #define TEST_WLZNMSUPRESS */
#ifdef TEST_WLZNMSUPRESS
int             main(int argc, char *argv[])
{
  WlzErrorNum   errNum = WLZ_ERR_NONE;
  WlzPixelV	sMinGrdV,
  		pMinGrdV;
  double        alpha,
  		mult;
  WlzObject     *inObj = NULL,
		*outObj = NULL;

  alpha = 1.0;
  mult = 10.0;
  sMinGrdV.type = WLZ_GREY_INT;
  sMinGrdV.v.inv = 100;
  pMinGrdV.type = WLZ_GREY_INT;
  pMinGrdV.v.inv = 400;
  if(((inObj = WlzAssignObject(WlzReadObj(stdin, &errNum), NULL)) == NULL) ||
     (errNum != WLZ_ERR_NONE))
  {
    (void )fprintf(stderr,
		   "%s: failed to read object from stdin\n",
		   *argv);
  }
  else if(((outObj = WlzCannyDeriche(NULL, inObj, alpha, mult,
  				     pMinGrdV, sMinGrdV,
  			             &errNum)) == NULL) ||
	  (errNum != WLZ_ERR_NONE))
  {
    (void )fprintf(stderr,
		   "%s: failed to Canny filter object\n",
		   *argv);
  }
  else if(WlzWriteObj(stdout, outObj) != WLZ_ERR_NONE)
  {
    (void )fprintf(stderr,
		   "%s: failed to write object\n",
		   *argv);
  }
  return(errNum);
}
#endif /* TEST_WLZNMSUPRESS */
