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
  libpf_global.h
  bspline/bspline2d.h
  bspline/def_bspline.h
  bspline/multibspline2d.h
  bspline/point2d.h
  colorutils/calclut.h
  colorutils/cvtcolor.h
  colorutils/cvtcoloralloc.h
  colsegmentation/ColorBase.h
  colsegmentation/ColorModel.h
  colsegmentation/skincolormodel.h
  colsegmentation/skincolorsegmentation.h
  colsegmentation/skinregionsegmentation.h
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
  distribution/gauss6d.txt
  dynamics/ContDynamicAlloc.h
  dynamics/ContDynamicModel.h
  dynamics/ContDynamicRW.h
  dynamics/DiscDynamicAlloc.h
  dynamics/DiscDynamicModel.h
  dynamics/ContDynamicList.txt
  dynamics/discdyn.txt
  dynamics/DiscDynamicList.txt
  dynamics/rw_monoDyn_XYTS.txt
  dynamics/test.txt
  filters/ICOND.h
  filters/ParticleFilter.h
  filters/WeightsVector.h
  imageutils/displayutils.h
  imageutils/histogrammes.h
  imageutils/imageutils.h
  imageutils/watershed.h
  imgproducer/ImgProducer.h
  likelihood/likelihoodutils.h
  likelihood/nok/Likelihood.h
  likelihood/nok/ShapeLikelihood.h
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
  miscelleanous/squarerootutils.h
  priors/Prior.h
  priors/PriorAlloc.h
  priors/PriorGauss.h
  priors/PriorGaussMixture.h
  priors/Sample.h
  priors/gauss2d.txt
  priors/gaussMix2d.txt
  priors/imgPrior_XY.txt
  priors/PriorList.txt
  shapedesc/shape.h
  shapedesc/ShapeModels.h
  shapedesc/shapeutils.h
  stringutils/stringutils.h
  stringutils/tmp.txt
  targets/Target.h
  targets/TargetAlloc.h
  targets/TargetICU.h
  targets/TargetList.txt
  timeutils/temps_cpu.h
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

INSTALL(FILES ${NEWFILES}
	DESTINATION include/LibPF
	PERMISSIONS OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE )