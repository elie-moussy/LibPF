# Copyright (C) LAAS-CNRS
# Author : Elie MOUSSY
#
# This file is part of LibPF.
# LibPF is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

SET(NEWFILES
  LibPF.h
  libpf_global.h)

INSTALL(FILES ${NEWFILES}
	DESTINATION include/LibPF
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(BSPLINEFILES
  bspline/bspline2d.h
  bspline/def_bspline.h
  bspline/multibspline2d.h
  bspline/point2d.h)

INSTALL(FILES ${BSPLINEFILES}
	DESTINATION include/LibPF/bspline
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(COLORUTILSFILES
  colorutils/calclut.h
  colorutils/cvtcolor.h
  colorutils/cvtcoloralloc.h)

INSTALL(FILES ${COLORUTILSFILES}
	DESTINATION include/LibPF/colorutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(COLSEGMENTATIONFILES
  colsegmentation/ColorBase.h
  colsegmentation/ColorModel.h
  colsegmentation/skincolormodel.h
  colsegmentation/skincolorsegmentation.h
  colsegmentation/skinregionsegmentation.h)

INSTALL(FILES ${COLSEGMENTATIONFILES}
	DESTINATION include/LibPF/colsegmentation
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(DISTRIBUTIONFILES
  distribution/ColorDistribution.h
  distribution/Gaussian.h
  distribution/Gaussian1D.h
  distribution/Gaussian2D.h
  distribution/Gaussian3D.h
  distribution/Gaussian4D.h
  distribution/GaussianAlloc.h
  distribution/GaussianDiag2D.h
  distribution/GaussianDiag3D.h
  distribution/GaussianDiag4D.h
  distribution/GaussianDiagND.h
  distribution/GaussianMixture.h
  distribution/GaussianND.h
  distribution/MotionDistribution.h
  distribution/gauss1d.txt
  distribution/gauss2d.txt
  distribution/gauss3d.txt
  distribution/gauss4d.txt
  distribution/gauss6d.txt)

INSTALL(FILES ${DISTRIBUTIONFILES}
	DESTINATION include/LibPF/distribution
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(DYNAMICSFILES
  dynamics/ContDynamicAlloc.h
  dynamics/ContDynamicModel.h
  dynamics/ContDynamicRW.h
  dynamics/DiscDynamicAlloc.h
  dynamics/DiscDynamicModel.h
  dynamics/ContDynamicList.txt
  dynamics/discdyn.txt
  dynamics/DiscDynamicList.txt
  dynamics/rw_monoDyn_XYTS.txt
  dynamics/test.txt)

INSTALL(FILES ${DYNAMICSFILES}
	DESTINATION include/LibPF/dynamics
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(FILTERSFILES
  filters/ICOND.h
  filters/ParticleFilter.h
  filters/WeightsVector.h)

INSTALL(FILES ${FILTERSFILES}
	DESTINATION include/LibPF/filters
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(IMAGEUTILSFILES
  imageutils/displayutils.h
  imageutils/histogrammes.h
  imageutils/imageutils.h
  imageutils/watershed.h)

INSTALL(FILES ${IMAGEUTILSFILES}
	DESTINATION include/LibPF/imageutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(IMGPRODUCERFILES
  imgproducer/ImgProducer.h)

INSTALL(FILES ${IMGPRODUCERFILES}
	DESTINATION include/LibPF/imgproducer
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(LIKELIHOODFILES
  likelihood/likelihoodutils.h)

INSTALL(FILES ${LIKELIHOODFILES}
	DESTINATION include/LibPF/likelihood
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(NOKFILES
  likelihood/nok/Likelihood.h
  likelihood/nok/ShapeLikelihood.h)

INSTALL(FILES ${NOKFILES}
	DESTINATION include/LibPF/likelihood/nok
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(MISCELLEANOUSFILES
  miscelleanous/applipath.h
  miscelleanous/constparams.h
  miscelleanous/cvwrap.h
  miscelleanous/def_paramslist.h
  miscelleanous/externParams.h
  miscelleanous/globalParams.h
  miscelleanous/matrixutils.h
  miscelleanous/paramslist.h
  miscelleanous/paramsrange.h
  miscelleanous/RoiManager.h
  miscelleanous/squarerootutils.h)

INSTALL(FILES ${MISCELLEANOUSFILES}
	DESTINATION include/LibPF/miscelleanous
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(PRIORSFILES
  priors/Prior.h
  priors/PriorAlloc.h
  priors/PriorGauss.h
  priors/PriorGaussMixture.h
  priors/Sample.h
  priors/gauss2d.txt
  priors/gaussMix2d.txt
  priors/imgPrior_XY.txt
  priors/PriorList.txt)

INSTALL(FILES ${PRIORSFILES}
	DESTINATION include/LibPF/priors
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(SHAPEDESCFILES
  shapedesc/shape.h
  shapedesc/ShapeModels.h
  shapedesc/shapeutils.h)

INSTALL(FILES ${SHAPEDESCFILES}
	DESTINATION include/LibPF/shapedesc
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(STRINGUTILSFILES
  stringutils/stringutils.h
  stringutils/tmp.txt)

INSTALL(FILES ${STRINGUTILSFILES}
	DESTINATION include/LibPF/stringutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TARGETSFILES
  targets/Target.h
  targets/TargetAlloc.h
  targets/TargetICU.h
  targets/TargetList.txt)

INSTALL(FILES ${TARGETSFILES}
	DESTINATION include/LibPF/targets
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TIMEUTILSFILES
  timeutils/temps_cpu.h)

INSTALL(FILES ${TIMEUTILSFILES}
	DESTINATION include/LibPF/timeutils
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )

SET(TWODDETECTORFILES
  twoDDetector/blobDetector.h
  twoDDetector/Detector.h
  twoDDetector/faceDetector.h
  twoDDetector/inputDetector.h
  twoDDetector/motionBlobDetector.h
  twoDDetector/motionDetector.h
  twoDDetector/skinBlobDetector.h
  twoDDetector/twoDDetector.h
  twoDDetector/twoDDetectorAlloc.h
  twoDDetector/HogPeopleDetector.h
  twoDDetector/twoDDetectorList.txt)

INSTALL(FILES ${TWODDETECTORFILES}
	DESTINATION include/LibPF/twoDDetector
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )