import sys
v=sys.argv
if v[1]=='row:
	title = v[2]
	thumb = v[3]
	link = v[4]
	print'''
                <div class="col-xs-6 col-sm-3 col-md-2">
                    <div class="recent-work-wrap">
                        <img class="img-responsive" src="images/portfolio/recent/%s" alt="">
                        <div class="overlay">
                            <div class="recent-work-inner">
                                <p>Study of AC circuits</p>
                                <a class="preview" href="images/portfolio/full/item1.png" rel="prettyPhoto"><i class="fa fa-eye"></i> View</a> <!-- optional for large image viewing in lightbox -->
                            </div> 
                        </div>
                        <div class="titleoverlay">
                            <div class="recent-work-inner">
                                <h3><a href="#">%s</a> </h3>
                            </div> 
                        </div>
                    </div>
                </div>   
	'''%(title,thumb,link)
elif v[1]=='slider':
	print'''
                <div class="col-xs-6 col-sm-3 col-md-2">
                    <div class="recent-work-wrap">
                        <img class="img-responsive" src="images/portfolio/recent/ac-circuits.png" alt="">
                        <div class="overlay">
                            <div class="recent-work-inner">
                                <p>Study of AC circuits</p>
                                <a class="preview" href="images/portfolio/full/item1.png" rel="prettyPhoto"><i class="fa fa-eye"></i> View</a> <!-- optional for large image viewing in lightbox -->
                            </div> 
                        </div>
                        <div class="titleoverlay">
                            <div class="recent-work-inner">
                                <h3><a href="#">AC Circuits</a> </h3>
                            </div> 
                        </div>
                    </div>
                </div>   
	'''
