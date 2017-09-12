"<!-- ******************************************************************** -->\n<!-- HPCToolkit Experiment DTD						  -->\n<!-- Version 2.1							  -->\n<!-- ******************************************************************** -->\n<!ELEMENT HPCToolkitExperiment (Header, (SecCallPathProfile|SecFlatProfile)*)>\n<!ATTLIST HPCToolkitExperiment\n	  version CDATA #REQUIRED>\n\n  <!-- ****************************************************************** -->\n\n  <!-- Info/NV: flexible name-value pairs: (n)ame; (t)ype; (v)alue -->\n  <!ELEMENT Info (NV*)>\n  <!ATTLIST Info\n	    n CDATA #IMPLIED>\n  <!ELEMENT NV EMPTY>\n  <!ATTLIST NV\n	    n CDATA #REQUIRED\n	    t CDATA #IMPLIED\n	    v CDATA #REQUIRED>\n\n  <!-- ****************************************************************** -->\n  <!-- Header								  -->\n  <!-- ****************************************************************** -->\n  <!ELEMENT Header (Info*)>\n  <!ATTLIST Header\n	    n CDATA #REQUIRED>\n\n  <!-- ****************************************************************** -->\n  <!-- Section Header							  -->\n  <!-- ****************************************************************** -->\n  <!ELEMENT SecHeader (MetricTable?, MetricDBTable?, TraceDBTable?, LoadModuleTable?, FileTable?, ProcedureTable?, Info*)>\n\n    <!-- MetricTable: -->\n    <!ELEMENT MetricTable (Metric)*>\n\n    <!-- Metric: (i)d; (n)ame -->\n    <!--   (v)alue-type: transient type of values -->\n    <!--   (t)ype: persistent type of metric -->\n    <!--   fmt: format; show; -->\n    <!ELEMENT Metric (MetricFormula*, Info?)>\n    <!ATTLIST Metric\n	      i            CDATA #REQUIRED\n	      n            CDATA #REQUIRED\n	      pes		   CDATA #IMPLIED\n	      pem		   CDATA #IMPLIED\n	      pep		   CDATA #IMPLIED\n	      v            (raw|final|derived-incr|derived) \"raw\"\n	      t            (inclusive|exclusive|nil) \"nil\"\n	      partner      CDATA #IMPLIED\n	      fmt          CDATA #IMPLIED\n	      show         (1|0) \"1\"\n	      show-percent (1|0) \"1\">\n\n    <!-- MetricFormula represents derived metrics: (t)ype; (frm): formula -->\n    <!ELEMENT MetricFormula (Info?)>\n    <!ATTLIST MetricFormula\n	      t   (combine|finalize) \"finalize\"\n	      i   CDATA #IMPLIED\n	      frm CDATA #REQUIRED>\n\n    <!-- Metric data, used in sections: (n)ame [from Metric]; (v)alue -->\n    <!ELEMENT M EMPTY>\n    <!ATTLIST M\n	      n CDATA #REQUIRED\n	      v CDATA #REQUIRED>\n\n    <!-- MetricDBTable: -->\n    <!ELEMENT MetricDBTable (MetricDB)*>\n\n    <!-- MetricDB: (i)d; (n)ame -->\n    <!--   (t)ype: persistent type of metric -->\n    <!--   db-glob:        file glob describing files in metric db -->\n    <!--   db-id:          id within metric db -->\n    <!--   db-num-metrics: number of metrics in db -->\n    <!--   db-header-sz:   size (in bytes) of a db file header -->\n    <!ELEMENT MetricDB EMPTY>\n    <!ATTLIST MetricDB\n	      i              CDATA #REQUIRED\n	      n              CDATA #REQUIRED\n	      t              (inclusive|exclusive|nil) \"nil\"\n	      partner        CDATA #IMPLIED\n	      db-glob        CDATA #IMPLIED\n	      db-id          CDATA #IMPLIED\n	      db-num-metrics CDATA #IMPLIED\n	      db-header-sz   CDATA #IMPLIED>\n\n    <!-- TraceDBTable: -->\n    <!ELEMENT TraceDBTable (TraceDB)>\n\n    <!-- TraceDB: (i)d -->\n    <!--   db-min-time: min beginning time stamp (global) -->\n    <!--   db-max-time: max ending time stamp (global) -->\n    <!ELEMENT TraceDB EMPTY>\n    <!ATTLIST TraceDB\n	      i            CDATA #REQUIRED\n	      db-glob      CDATA #IMPLIED\n	      db-min-time  CDATA #IMPLIED\n	      db-max-time  CDATA #IMPLIED\n	      db-header-sz CDATA #IMPLIED>\n\n    <!-- LoadModuleTable assigns a short name to a load module -->\n    <!ELEMENT LoadModuleTable (LoadModule)*>\n\n    <!ELEMENT LoadModule (Info?)>\n    <!ATTLIST LoadModule\n	      i CDATA #REQUIRED\n	      n CDATA #REQUIRED>\n\n    <!-- FileTable assigns a short name to a file -->\n    <!ELEMENT FileTable (File)*>\n\n    <!ELEMENT File (Info?)>\n    <!ATTLIST File\n	      i CDATA #REQUIRED\n	      n CDATA #REQUIRED>\n\n    <!-- ProcedureTable assigns a short name to a procedure -->\n    <!ELEMENT ProcedureTable (Procedure)*>\n\n    <!ELEMENT Procedure (Info?)>\n    <!ATTLIST Procedure\n	      i CDATA #REQUIRED\n	      n CDATA #REQUIRED>\n\n  <!-- ****************************************************************** -->\n  <!-- Section: Call path profile					  -->\n  <!-- ****************************************************************** -->\n  <!ELEMENT SecCallPathProfile (SecHeader, SecCallPathProfileData)>\n  <!ATTLIST SecCallPathProfile\n	    i CDATA #REQUIRED\n	    n CDATA #REQUIRED>\n\n    <!ELEMENT SecCallPathProfileData (PF|M)*>\n      <!-- Procedure frame -->\n      <!--   (i)d: unique identifier for cross referencing -->\n      <!--   (s)tatic scope id -->\n      <!--   (n)ame: a string or an id in ProcedureTable -->\n      <!--   (lm) load module: a string or an id in LoadModuleTable -->\n      <!--   (f)ile name: a string or an id in LoadModuleTable -->\n      <!--   (l)ine range: \"beg-end\" (inclusive range) -->\n      <!--   (a)lien: whether frame is alien to enclosing P -->\n      <!--   (str)uct: hpcstruct node id -->\n      <!--   (v)ma-range-set: \"{[beg-end), [beg-end)...}\" -->\n      <!ELEMENT PF (PF|Pr|L|C|S|M)*>\n      <!ATTLIST PF\n		i  CDATA #IMPLIED\n		s  CDATA #IMPLIED\n		n  CDATA #REQUIRED\n		lm CDATA #IMPLIED\n		f  CDATA #IMPLIED\n		l  CDATA #IMPLIED\n		str  CDATA #IMPLIED\n		v  CDATA #IMPLIED>\n      <!-- Procedure (static): GOAL: replace with 'P' -->\n      <!ELEMENT Pr (Pr|L|C|S|M)*>\n      <!ATTLIST Pr\n                i  CDATA #IMPLIED\n		s  CDATA #IMPLIED\n                n  CDATA #REQUIRED\n		lm CDATA #IMPLIED\n		f  CDATA #IMPLIED\n                l  CDATA #IMPLIED\n		a  (1|0) \"0\"\n		str  CDATA #IMPLIED\n		v  CDATA #IMPLIED>\n      <!-- Callsite (a special StatementRange) -->\n      <!ELEMENT C (PF|M)*>\n      <!ATTLIST C\n		i CDATA #IMPLIED\n		s CDATA #IMPLIED\n		l CDATA #IMPLIED\n		str CDATA #IMPLIED\n		v CDATA #IMPLIED>\n\n  <!-- ****************************************************************** -->\n  <!-- Section: Flat profile						  -->\n  <!-- ****************************************************************** -->\n  <!ELEMENT SecFlatProfile (SecHeader, SecFlatProfileData)>\n  <!ATTLIST SecFlatProfile\n	    i CDATA #REQUIRED\n	    n CDATA #REQUIRED>\n\n    <!ELEMENT SecFlatProfileData (LM|M)*>\n      <!-- Load module: (i)d; (n)ame; (v)ma-range-set -->\n      <!ELEMENT LM (F|P|M)*>\n      <!ATTLIST LM\n                i CDATA #IMPLIED\n                n CDATA #REQUIRED\n		v CDATA #IMPLIED>\n      <!-- File -->\n      <!ELEMENT F (P|L|S|M)*>\n      <!ATTLIST F\n                i CDATA #IMPLIED\n                n CDATA #REQUIRED>\n      <!-- Procedure (Note 1) -->\n      <!ELEMENT P (P|A|L|S|C|M)*>\n      <!ATTLIST P\n                i CDATA #IMPLIED\n                n CDATA #REQUIRED\n                l CDATA #IMPLIED\n		str CDATA #IMPLIED\n		v CDATA #IMPLIED>\n      <!-- Alien (Note 1) -->\n      <!ELEMENT A (A|L|S|C|M)*>\n      <!ATTLIST A\n                i CDATA #IMPLIED\n                f CDATA #IMPLIED\n                n CDATA #IMPLIED\n                l CDATA #IMPLIED\n		str CDATA #IMPLIED\n		v CDATA #IMPLIED>\n      <!-- Loop (Note 1,2) -->\n      <!ELEMENT L (A|Pr|L|S|C|M)*>\n      <!ATTLIST L\n		i CDATA #IMPLIED\n		s CDATA #IMPLIED\n		l CDATA #IMPLIED\n	        f CDATA #IMPLIED\n		str CDATA #IMPLIED\n		v CDATA #IMPLIED>\n      <!-- Statement (Note 2) -->\n      <!--   (it): trace record identifier -->\n      <!ELEMENT S (S|M)*>\n      <!ATTLIST S\n		i  CDATA #IMPLIED\n		it CDATA #IMPLIED\n		s  CDATA #IMPLIED\n		l  CDATA #IMPLIED\n		str  CDATA #IMPLIED\n		v  CDATA #IMPLIED>\n      <!-- Note 1: Contained Cs may not contain PFs -->\n      <!-- Note 2: The 's' attribute is not used for flat profiles -->\n";
