### TODO list:

1. **No tests**:
    - There are no unit or integration tests, which makes it more difficult to verify the correctness of the code.

1. **No CORS support**:
    - If the API is going to be used in web applications, you need to add CORS headers.  

1. **There is no load on the SSL context**:
    - Certificates and keys are downloaded from files, but there is no verification of their validity or support for the update mechanism.  

1. **Weak API documentation**:
    - There is no Swagger/OpenAPI specification or detailed README describing all endpoints.  

1. **No load balancing and scaling**:  
    - The server is not designed for horizontal scaling.
---
### Done:
    
1. **There is no processing of duplicate users**:
    - In the "authorization_service" there is no verification of the uniqueness of username and email before insertion into the database, which can lead to errors.  

1. **Lack of an authentication mechanism**:
    - There is registration, but there is no login (login) and token issuance (JWT, sessions, etc.).  

1. **There is no limit on the request size**:  
    - The buffer for reading an HTTP request is unlimited, which can lead to a memory exhaustion attack.  

1. **The absence of a graceful shutdown**:
    - There is no mechanism for correctly shutting down the server (for example, when receiving a SIGINT).  

1. **Insufficient logging**:  
    - Only errors and basic events are logged, but there is no detailed logging (for example, successful registrations).  

1. **Logging user actions**
    - A table with the history of logins, password changes, etc.

1. **No password length validation**:
    - The password is not checked for minimum length or complexity.  

1. **Endpoint**
    - Endpoint /health for checking state BD and services.

1. **Roles**
    - Add the role field to the user table.
