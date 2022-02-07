.. # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

#######################
Source Code Annotations
#######################

Users can annotate their workflow code to get end-to-end performance insights.
Currently, three techniques are available for this:
- Critical path annotation
- Synchronous events annotation
- Asynchronous events annotation

For critical path annotation, the user can provide pointcut and scope 
information for the annotated region. Currently, valid pointcut values are 
`before`, `after`, `around`, `before_async`, `after_async`, and `around_async`. 
We show an example below:


For synchronous event annotation, the user can provide a pointcut, name,
and category for the annotated region. Valid pointcut values are `before`,
`after`, and `around`. The name represents a way to identify the current function
being annotated, and the category can be a filename. 
An example of this is shown below:

.. code-block:: python   

    #! /usr/bin/python3                                                                
                                                                                   
    import time                                                                        
    import os.path                                                                     
    from perfflowaspect import aspect                                                  
                                                                                   
    def foo():                                                                         
        aspect.sync_event("before", "foo", filename)                                   
        time.sleep(2)                                                                  
        print("hello")                                                                 
        aspect.sync_event("after", "foo", filename)                                    
                                                                                   
    def main():                                                                        
        aspect.sync_event("before", "main", filename)                                  
        foo()                                                                          
        aspect.sync_event("after", "main", filename)                                   
                                                                                   
    if __name__ == "__main__":                                                         
        filename = os.path.basename(__file__)                                          
        main() 


For asynchronous event annotation, the user can provide a pointcut, name,
category, and scopr for the annotated region. An example of this is shown below:



(TBD: Add more detail about each of the pointcuts and default values, etc.)
