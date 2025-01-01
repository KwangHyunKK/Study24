# Methods for Changing Docker Container Volumes

This document summarizes **methods to add or change volumes in Docker containers**. It also includes ways to perform these tasks without deleting existing containers.

---

## **1. Adding a Bind Mount (Temporary Mount)**
You can add a temporary mount to a running container without deleting it.

### **Steps**
1. **Check the container PID**
   ```
   docker inspect -f '{{ .State.Pid }}' <container_name>
   ```
2. **Enter the container namespace**
   ```
   nsenter --target <PID> --mount --uts --ipc --net --pid
   ```
3. **Add the mount**
   ```
   mount --bind /host/path /container/path
   ```
4. **Verify the mount**
   ```
   df -h
   ```
> **Note:** The mount will be removed if the container restarts.

**Example:**
Assume you have a running container named `web_container` and you want to temporarily mount the host directory `/host_data` to `/container_data` inside the container.
```bash
docker inspect -f '{{ .State.Pid }}' web_container
nsenter --target <PID> --mount --uts --ipc --net --pid
mount --bind /host_data /container_data
```
This approach is useful for debugging or temporary data sharing.

---

## **2. Using a Volume Container for Data Sharing**
Dynamic mounts are complex, but you can configure data sharing using a volume container.

### **Steps**
1. **Create a new volume**
   ```
   docker volume create new_volume
   ```
2. **Attach the volume to a data container**
   ```
   docker run -v new_volume:/data --name data_container busybox
   ```
3. **Use the data container in an existing container**
   ```
   docker run --volumes-from data_container -d <image>
   ```

**Example:**
If you have a web application in a container that needs access to shared data:
```bash
docker volume create web_data
docker run -v web_data:/data --name data_container busybox
docker run --volumes-from data_container -d nginx
```
This method maintains persistent data storage across multiple containers.

---

## **3. Adding a Volume Using Docker Commit**
You can retain the current container state while creating a new image with additional volumes.

### **Steps**
1. **Save the current state as an image**
   ```
   docker commit <container_name_or_id> new_image_name
   ```
2. **Create a new container with the volume added**
   ```
   docker run -d --name <new_container_name> -v /new/host/path:/container/path new_image_name
   ```
> Data migration may be required.

**Example:**
Assume you want to add a volume to a running `app_container` without losing its state:
```
docker commit app_container app_image_with_volume
docker run -d --name app_container_v2 -v /new/volume:/app/data app_image_with_volume
```
This approach is ideal for making permanent changes to the container configuration.

---

## **4. Adding Docker Volume Mounts Using Plugins**
Docker volume plugins allow **dynamic mounts without restarting containers**.
- Example plugins: `rexray` or `local-persist`.

**Example:**
```
docker plugin install rexray/s3fs
rexray volume create example-volume
```
This example demonstrates how to use `rexray` to dynamically add cloud-based storage without container restarts.

---

## **Conclusion**
When working with Docker volumes, you can choose from several approaches based on your needs:

1. Use **bind mounts** for temporary changes, which are ideal for debugging or quick file sharing.
2. Use **volume containers** to share data persistently across multiple containers.
3. Use **Docker commit** to create permanent configurations without losing container state.
4. Use **plugins** for advanced features like dynamic mounting and external storage integration.
