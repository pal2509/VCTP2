# Licence-plate recognition in C, from scratch

A course project for Computer Vision (Visão por Computador) in my Bachelor's in Information
Systems Engineering at IPCA, written in May and June 2020. It finds Portuguese licence plates
in a video, frame by frame, draws a box around the plate and each character, and reads the
characters — using only image-processing functions written by hand in C. OpenCV is used for
nothing but reading the video and showing the window.

## How it works

Each frame goes through `Frame()` in `CodigoExemplo.cpp`:

1. **Find the plate's rows.** The frame is converted to grayscale and scanned in horizontal
   lines. A plate shows up as a line with many sharp light/dark changes close together (the
   characters against the white background), so a line with 6 to 13 such contrast jumps, all
   near each other, is taken as crossing the plate. Scanning up and down from there gives its
   top and bottom.
2. **Find the plate itself.** That strip is binarised with Bernsen's local threshold (the
   kernel scaled to the strip's width, the contrast limit from its mean and standard
   deviation), cleaned with a morphological opening, and labelled into blobs. The plate is
   the blob with a 4:1 to 5:1 aspect ratio; a column histogram cross-checks its left and
   right edges.
3. **Split the characters.** The plate is binarised again, inverted and labelled; blobs with a
   character's proportions and size become the six characters.
4. **Read each character.** `IdentificaCaracter()` counts colour changes along three
   horizontal lines and one vertical line through the character, and the peaks in its
   smoothed row and column histograms. A set of rules maps those counts to a character.

`vc.c` / `vc.h` is the image-processing library built during the course on the professor's
skeleton: image I/O, RGB/HSV/gray conversions, global, midpoint and Bernsen thresholding,
binary and grayscale morphology, Prewitt edges, and connected-component labelling with blob
area, bounding box, centroid and perimeter.

## Run it

Windows only, as submitted: open `OpenCVTP.sln` in Visual Studio 2019 (toolset v142) with
OpenCV 4.3 installed at `C:\opencv` (the project links `opencv_world430.lib`). It plays
`VC-TP2 - Enunciado\Videos\video1.mp4`; change `videofile` in `main()` for the other video
or a webcam. `q` pauses. The source files are Latin-1 encoded and the comments are in
Portuguese.

## Honest limits

- **It reads only what the test videos needed.** The rules recognise the characters on the
  plates in the two course videos (2, 6, 7, 8, 9, Q, R and U), tuned by hand. For anything
  else `IdentificaCaracter()` falls off the end without returning a value.
- **The thresholds are magic numbers** fitted to those videos: line spacing, contrast, blob
  ratios. Other cameras, distances or lighting would need retuning.
- **Memory bugs I'd catch today.** Images are released with `free()` instead of
  `vc_image_free()`, so every frame leaks its pixel buffers. `Frame()` frees the plate string
  that `main()` then prints, and that string has no room for its terminating `\0`.

The classic answer today would be a trained detector for the plate and OCR for the
characters; this project is the version before any of that, built from pixels up.

## Licence

`vc.c` and `vc.h` started from the course's skeleton (IPCA, Duarte Duque), and the files in
`VC-TP2 - Enunciado/` are the course's assignment, sample code, reading material and videos.
They are here for context and are not mine to licence.
