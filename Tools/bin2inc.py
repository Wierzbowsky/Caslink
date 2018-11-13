#
# Bin to Inc utility for Caslink project. Version 1.0
# Converts a binary file to an array of hex values (comma-separated with ident)
# Copyright (C) 2016 by Alexey Podrezov
#

import sys
import os
import codecs

if __name__ == "__main__":

	try:
		print ("\nBIN2INC Utility for Caslink Project. Version 1.0")
		print ("Converts a binary file to an array of hex values (comma-separated with ident)")
		print ("Copyright (C) 2016 by Alexey Podrezov\n")

		if len(sys.argv) < 2:
			print ("Usage: bin2inc inputfile outputfile\n")
			sys.exit()
		elif len(sys.argv) == 3 and os.path.isfile(sys.argv[1]):

			try:
				hinput = open (sys.argv[1], "rb")
				houtput = open (sys.argv[2], "w")
			except IOError:
				print ("Failed to open or create a file...")
				sys.exit()

			fsize = os.path.getsize(sys.argv[1])
			if fsize == 0:
				print ("Input file is empty...")
				sys.exit()

			try:
				houtput.write("\t")
			except IOError:
				print ("Failed to write output file...")
				sys.exit()

			ident = 0
			while fsize:
				try:
					fbyte = hinput.read(1)
				except IOError:
					print ("Failed to read input file...")
					sys.exit()
			
				try:
					houtput.write("0x%02X" % int.from_bytes(fbyte, byteorder='big', signed=False))
					if fsize > 1:
						houtput.write(", ")
				except IOError:
					print ("Failed to write output file...")
					sys.exit()

				ident = ident + 1
				if ident > 24:
					try:
						houtput.write("\n\t")
					except IOError:
						print ("Failed to write output file...")
						sys.exit()
					ident = 0	

				fsize = fsize - 1

			hinput.close()
			houtput.close()

			print ("All done! Check the %s file." % sys.argv[2])
			sys.exit()

		else:
			print ("Invalid parameter(s) passed to command line...")
			sys.exit()

	except KeyboardInterrupt:
		print ("Program terminated by user...")
		sysexit()