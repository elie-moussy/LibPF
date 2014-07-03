# Copyright (C) LAAS-CNRS
# Author : Elie MOUSSY
#
# This file is part of LibPF.
# LibPF is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

SET(NEWFILES
  LibPF/LibPF.h
  LibPF/libpf_global.h)

INSTALL(FILES ${NEWFILES}
	DESTINATION include/LibPF
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(BSPLINEFILES
  LibPF/bspline/bspline2d.h
  LibPF/bspline/def_bspline.h
  LibPF/bspline/multibspline2d.h
  LibPF/bspline/point2d.h)

INSTALL(FILES ${BSPLINEFILES}
	DESTINATION include/LibPF/bspline
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(COLORUTILSFILES
  LibPF/colorutils/calclut.h
  LibPF/colorutils/cvtcolor.h
  LibPF/colorutils/cvtcoloralloc.h)

INSTALL(FILES ${COLORUTILSFILES}
	DESTINATION include/LibPF/colorutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(COLSEGMENTATIONFILES
  LibPF/colsegmentation/ColorBase.h
  LibPF/colsegmentation/ColorModel.h
  LibPF/colsegmentation/skincolormodel.h
  LibPF/colsegmentation/skincolorsegmentation.h
  LibPF/colsegmentation/skinregionsegmentation.h)

INSTALL(FILES ${COLSEGMENTATIONFILES}
	DESTINATION include/LibPF/colsegmentation
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(DISTRIBUTIONFILES
  LibPF/distribution/ColorDistribution.h
  LibPF/distribution/Gaussian.h
  LibPF/distribution/Gaussian1D.h
  LibPF/distribution/Gaussian2D.h
  LibPF/distribution/Gaussian3D.h
  LibPF/distribution/Gaussian4D.h
  LibPF/distribution/GaussianAlloc.h
  LibPF/distribution/GaussianDiag2D.h
  LibPF/distribution/GaussianDiag3D.h
  LibPF/distribution/GaussianDiag4D.h
  LibPF/distribution/GaussianDiagND.h
  LibPF/distribution/GaussianMixture.h
  LibPF/distribution/GaussianND.h
  LibPF/distribution/MotionDistribution.h
  LibPF/distribution/gauss1d.txt
  LibPF/distribution/gauss2d.txt
  LibPF/distribution/gauss3d.txt
  LibPF/distribution/gauss4d.txt
  LibPF/distribution/gauss6d.txt)

INSTALL(FILES ${DISTRIBUTIONFILES}
	DESTINATION include/LibPF/distribution
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(DYNAMICSFILES
  LibPF/dynamics/ContDynamicAlloc.h
  LibPF/dynamics/ContDynamicModel.h
  LibPF/dynamics/ContDynamicRW.h
  LibPF/dynamics/DiscDynamicAlloc.h
  LibPF/dynamics/DiscDynamicModel.h
  LibPF/dynamics/ContDynamicList.txt
  LibPF/dynamics/discdyn.txt
  LibPF/dynamics/DiscDynamicList.txt
  LibPF/dynamics/rw_monoDyn_XYTS.txt
  LibPF/dynamics/test.txt)

INSTALL(FILES ${DYNAMICSFILES}
	DESTINATION include/LibPF/dynamics
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(FILTERSFILES
  LibPF/filters/ICOND.h
  LibPF/filters/ParticleFilter.h
  LibPF/filters/WeightsVector.h)

INSTALL(FILES ${FILTERSFILES}
	DESTINATION include/LibPF/filters
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(IMAGEUTILSFILES
  LibPF/imageutils/displayutils.h
  LibPF/imageutils/histogrammes.h
  LibPF/imageutils/imageutils.h
  LibPF/imageutils/watershed.h)

INSTALL(FILES ${IMAGEUTILSFILES}
	DESTINATION include/LibPF/imageutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(IMGPRODUCERFILES
  LibPF/imgproducer/ImgProducer.h)

INSTALL(FILES ${IMGPRODUCERFILES}
	DESTINATION include/LibPF/imgproducer
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(LIKELIHOODFILES
  LibPF/likelihood/likelihoodutils.h)

INSTALL(FILES ${LIKELIHOODFILES}
	DESTINATION include/LibPF/likelihood
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(NOKFILES
  LibPF/likelihood/nok/Likelihood.h
  LibPF/likelihood/nok/ShapeLikelihood.h)

INSTALL(FILES ${NOKFILES}
	DESTINATION include/LibPF/likelihood/nok
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(MISCELLEANOUSFILES
  LibPF/miscelleanous/applipath.h
  LibPF/miscelleanous/constparams.h
  LibPF/miscelleanous/cvwrap.h
  LibPF/miscelleanous/def_paramslist.h
  LibPF/miscelleanous/externParams.h
  LibPF/miscelleanous/globalParams.h
  LibPF/miscelleanous/matrixutils.h
  LibPF/miscelleanous/paramslist.h
  LibPF/miscelleanous/paramsrange.h
  LibPF/miscelleanous/RoiManager.h
  LibPF/miscelleanous/squarerootutils.h)

INSTALL(FILES ${MISCELLEANOUSFILES}
	DESTINATION include/LibPF/miscelleanous
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(PRIORSFILES
  LibPF/priors/Prior.h
  LibPF/priors/PriorAlloc.h
  LibPF/priors/PriorGauss.h
  LibPF/priors/PriorGaussMixture.h
  LibPF/priors/Sample.h
  LibPF/priors/gauss2d.txt
  LibPF/priors/gaussMix2d.txt
  LibPF/priors/imgPrior_XY.txt
  LibPF/priors/PriorList.txt)

INSTALL(FILES ${PRIORSFILES}
	DESTINATION include/LibPF/priors
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(SHAPEDESCFILES
  LibPF/shapedesc/shape.h
  LibPF/shapedesc/ShapeModels.h
  LibPF/shapedesc/shapeutils.h)

INSTALL(FILES ${SHAPEDESCFILES}
	DESTINATION include/LibPF/shapedesc
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(STRINGUTILSFILES
  LibPF/stringutils/stringutils.h
  LibPF/stringutils/tmp.txt)

INSTALL(FILES ${STRINGUTILSFILES}
	DESTINATION include/LibPF/stringutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TARGETSFILES
  LibPF/targets/Target.h
  LibPF/targets/TargetAlloc.h
  LibPF/targets/TargetICU.h
  LibPF/targets/TargetList.txt)

INSTALL(FILES ${TARGETSFILES}
	DESTINATION include/LibPF/targets
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TIMEUTILSFILES
  LibPF/timeutils/temps_cpu.h)

INSTALL(FILES ${TIMEUTILSFILES}
	DESTINATION include/LibPF/timeutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TWODDETECTORFILES
  LibPF/twoDDetector/blobDetector.h
  LibPF/twoDDetector/Detector.h
  LibPF/twoDDetector/faceDetector.h
  LibPF/twoDDetector/inputDetector.h
  LibPF/twoDDetector/motionBlobDetector.h
  LibPF/twoDDetector/motionDetector.h
  LibPF/twoDDetector/skinBlobDetector.h
  LibPF/twoDDetector/twoDDetector.h
  LibPF/twoDDetector/twoDDetectorAlloc.h
  LibPF/twoDDetector/HogPeopleDetector.h
  LibPF/twoDDetector/twoDDetectorList.txt)

INSTALL(FILES ${TWODDETECTORFILES}
	DESTINATION include/LibPF/twoDDetector
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )