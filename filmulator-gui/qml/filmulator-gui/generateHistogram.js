function generateHistogram(histNumber,ctx,width,height,padding,lineWidth,uiScale)
{
    var histFunc;
    switch (histNumber)
    {
    case 1:
        histFunc = function(a,b) {return filmProvider.getHistFinalPoint(a,b);};
        break;
    case 2:
        histFunc = function(a,b) {return filmProvider.getHistPreFilmPoint(a,b);};
        break;
    case 3:
        histFunc = function(a,b) {return filmProvider.getHistPostFilmPoint(a,b);};
    }

    ctx.save();
    ctx.clearRect(0, 0, width, height);
    ctx.globalAlpha = 1;
    ctx.lineWidth = lineWidth;
    var myGradient = ctx.createLinearGradient(0,0,width,0);

    var startx = padding;
    var endx = width - padding;
    var graphwidth = endx - startx;
    var starty = height - padding;
    var endy = padding+1*uiScale;
    var graphheight = starty - endy;
    var histPoint = 0;
    var maxValue = 128.0

    //Luma curve
    ctx.beginPath();
    ctx.moveTo(startx,starty);
    for(var i = 0; i < maxValue; i++)
    {
        histPoint = histFunc(0,i);
        ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
    }
    ctx.lineTo(endx,starty);
    ctx.lineTo(startx,starty);
    ctx.closePath();
    myGradient.addColorStop(1,"white");
    myGradient.addColorStop(0,'rgb(180,180,180)');
    ctx.fillStyle = myGradient;
    ctx.fill()

    //rCurve
    ctx.beginPath()
    ctx.moveTo(startx,starty);
    for(var i = 0; i < maxValue; i++)
    {
        histPoint = histFunc(1,i);
        ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
    }
    ctx.lineTo(endx,starty);
    ctx.closePath();
    ctx.strokeStyle = "#FF0000";
    ctx.stroke();

    //gCurve
    ctx.beginPath()
    ctx.moveTo(startx,starty);
    for(var i = 0; i < maxValue; i++)
    {
        histPoint = histFunc(2,i);
        ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
    }
    ctx.lineTo(endx,starty);
    ctx.closePath();
    ctx.strokeStyle = "#00FF00";
    ctx.stroke();

    //bCurve
    ctx.beginPath()
    ctx.moveTo(startx,starty);
    for(var i = 0; i < maxValue; i++)
    {
        histPoint = histFunc(3,i);
        ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
    }
    ctx.lineTo(endx,starty);
    ctx.closePath();
    ctx.strokeStyle = "#0000FF"
    ctx.stroke();

    ctx.strokeStyle = "#000000";
    ctx.strokeRect(startx,endy-1*uiScale,graphwidth,graphheight+1*uiScale);

    ctx.restore();
}
