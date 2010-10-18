<?php
	/* definitions */

	error_reporting(E_WARNING);

	define ("GWIDTH", 900);
	define ("GHEIGHT", 350);
	define ("XLIMIT", 9);
	define ("XSPACING", 40);
	define ("FILE", "/usr/local/apache/htdocs/proxy_framework/image.png");

	/* graph functions */

	function drawExisting() {
            header("Content-type: image/png");
            $im = @imagecreatefrompng(FILE) or die("Cannot Initialize image from file");
            imagepng($im);
            imagedestroy($im);
            die;							    
	}

	function drawGraph($x, $y, $w = GWIDTH, $h = GHEIGHT) {		
		$im = @imagecreate($w, $h)
		    or die("Cannot Initialize new GD image stream");
		$backgroundColor = imagecolorallocate($im, 255, 255, 255);

		$legend = array(
				"Bad" => imagecolorallocate($im, 255, 0, 0),
				"Good" => imagecolorallocate($im, 51, 255, 255),
				"SSL" => imagecolorallocate($im, 250, 230, 0),
				"Fast" => imagecolorallocate($im, 0, 255, 0)
		);

		drawAxis($im, $w, $h, $x, $y, $legend);
		imagepng($im, FILE);
		imagedestroy($im);		
	}

	function trimXAxis($axis, &$pos, $w, $endPadding, $startPadding, $xAxisEndPadding) {
		$xPos = $startPadding;
		$xDivisions = count($axis);
		$xMod = ($w - $xAxisEndPadding - $startPadding)/$xDivisions;

		if (count($axis) > XLIMIT) {
			$cmp = ceil(count($axis)/XLIMIT);
		} else {
			for ($i = 0; $i < count($axis); $i++)	{
				$pos[$i] = $xPos;
				$xPos += $xMod;
			}

			return $axis;
		}

		$newAxis = array();
		$left = XLIMIT - 2;

		for ($i = 0; $i < count($axis); $i++)	{
			// first and last ALWAYS get counted
			if (($i % $cmp == 0 && $left > 0) || ($i == count($axis) - 1))	{
				if ($i != 0 && $i != count($axis) - 1) {
					$left--;
				}

				$pos[] = $xPos;
				$newAxis[] = $axis[$i];				
			}
			$xPos += $xMod;
		}

		return $newAxis;
	}

	function drawAxis(&$im, $w, $h, $xAxis, $yAxis, $legend) {		
		$yAxisColor = imagecolorallocate($im, 233, 14, 91);
		$xAxisColor = imagecolorallocate($im, 14, 91, 233);
		$lineColor = imagecolorallocate($im, 0, 0, 0);

		// draw the axis
		$endPadding = 10;
		$startPadding = 30;
		$xAxisEndPadding = 60;

		// xAxis
		imageline($im, $startPadding, $endPadding, $w - $xAxisEndPadding, $endPadding, $lineColor);
		imageline($im, $startPadding, $h - $startPadding, $w - $xAxisEndPadding, $h - $startPadding, $lineColor);
		// yAxis
		imageline($im, $startPadding, $h - $startPadding, $startPadding, $endPadding, $lineColor);
		imageline($im, $w - $xAxisEndPadding, $h - $startPadding, $w - $xAxisEndPadding, $endPadding, $lineColor);

		// draw outline
		imageline($im, 1, 1, GWIDTH - 1, 1, $lineColor);
		imageline($im, 1, GHEIGHT - 1, GWIDTH - 1, GHEIGHT - 1, $lineColor);
		imageline($im, 1, 1, 1, GHEIGHT - 1, $lineColor);
		imageline($im, GWIDTH - 1, 1, GWIDTH - 1, GHEIGHT - 1, $lineColor);		

		// timestamp
                imagestring($im, 1, $w - $xAxisEndPadding + 8, $endPadding + 30, date("m-d-y"), $lineColor);
		imagestring($im, 1, $w - $xAxisEndPadding + 8, $endPadding + 40, date("H:i:s"), $lineColor);

                // legend
		$legendY = $endPadding + 70;
		foreach ($legend as $key => $color) {
                    imagestring($im, 1, $w - $xAxisEndPadding + 10, $legendY, $key, $lineColor);
	            drawLine($im, $w - $xAxisEndPadding + 10 + 24, $legendY + 5, $w - $xAxisEndPadding + 10 + 37, $legendY + 5, $color);
	            $legendY += 10;
		}
		$legendY -= 10;

		
		$xAxisColor = imagecolorallocate($im, 14, 91, 233);		

		$minY = 9999999;
		$maxY = -9999999;             

                $axis = array();
		$origYaxis = $yAxis;
		for ($i = 0; $i < count($yAxis); $i++) {
		    $axis = array_merge($axis, $yAxis[$i]);
		}
		$yAxis = $axis;
			
		foreach ($yAxis as $v)	{
			if ($minY > $v) 
				$minY = $v;
			if ($maxY < $v) 
				$maxY = $v;			
		}		

		$yDivisions = 5;
		$yPos = $h - $startPadding;
		for ($i = $minY; floor($i) <= $maxY; $i += ($maxY - $minY)/$yDivisions) {			
			imageline($im, $startPadding - 5, $yPos, $startPadding + 5, $yPos, $lineColor);
			imagestring($im, 1, $startPadding - 25, $yPos - 5, round($i), $lineColor);
			$yPos -= ($h - 2*$startPadding)/$yDivisions;
		}

		$xPos = $startPadding;
		$xDivisions = count($xAxis);
		
		$trimmedXAxis = trimXAxis($xAxis, $trimmedXPos, $w, $endPadding, $startPadding, $xAxisEndPadding);
		foreach ($trimmedXAxis as $k => $v) {
			imageline($im, $trimmedXPos[$k], $h - $startPadding - 5, $trimmedXPos[$k], $h - $startPadding + 5, $lineColor);
			if (($k + 1 < count($trimmedXAxis) && $trimmedXPos[$k] >= $trimmedXPos[$k + 1] - XSPACING) || $trimmedXPos[$k] >= $w - XSPACING) {
				$trimmedXPos[$k] -= 25;
			}
			imagestring($im, 1, $trimmedXPos[$k], $h - $startPadding + 10, $v, $xAxisColor);			
		}

		// TODO: support for n yAxises here
		$i = 0;
		foreach ($legend as $key => $color) {
		    plotPoints($im, $w, $h, $startPadding, $endPadding, $origYaxis[$i++], $minY, $maxY, $xDivisions, $yDivisions, $color, $xAxisEndPadding);		
		}
		/*
		plotPoints($im, $w, $h, $startPadding, $endPadding, $yAxisUsd, $minY, $maxY, $xDivisions, $yDivisions, $usdLineColor, $xAxisEndPadding);
		plotPoints($im, $w, $h, $startPadding, $endPadding, $yAxisEur, $minY, $maxY, $xDivisions, $yDivisions, $eurLineColor, $xAxisEndPadding);
		plotPoints($im, $w, $h, $startPadding, $endPadding, $yAxisRon, $minY, $maxY, $xDivisions, $yDivisions, $ronLineColor, $xAxisEndPadding);
		*/
	}

	function drawPoint(&$im, $x, $y, $pointColor) {
		imagefilledellipse($im, $x, $y, 6, 6, $pointColor);
	}

	function drawLine(&$im, $x1, $y1, $x2, $y2, $lineColor) {
		imageline($im, $x1, $y1, $x2, $y2, $lineColor);
	}

	function plotPoints(&$im, $w, $h, $startPadding, $endPadding, $yAxis, $minY, $maxY, $xDivisions, $yDivisions, $lineColor, $xAxisEndPadding) {
		$x1 = $startPadding;
		$xMod = ($w - $xAxisEndPadding - $startPadding)/$xDivisions;

		for ($i = 0; $i < count($yAxis); $i++)	{			 
			$y1 = $h - $startPadding - ($h - 2*$startPadding)*($yAxis[$i] - $minY)/($maxY - $minY);

			drawPoint($im, $x1, $y1, $lineColor);

			if ($i + 1 < count($yAxis)) {
				$x2 = $x1 + $xMod;
				$y2 = $h - $startPadding - ($h - 2*$startPadding)*($yAxis[$i + 1] - $minY)/($maxY - $minY);
				drawLine($im, $x1, $y1, $x2, $y2, $lineColor);
			}

			$x1 += $xMod;
		}		
	}
?>
