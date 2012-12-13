{
	int ii;
	for (ii = 1; ii < argc; ii++) {
		if (argu[ii][0] != '-' || argu[ii][1] == 0) break;
		if (argu[ii][1] == '-' && argu[ii][2] == 0) {
			ii++;
			break;
		}
		for (int jj = 1; argu[ii][jj]; jj++) switch (argu[ii][jj]) {
