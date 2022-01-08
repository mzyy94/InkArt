<script lang="ts">
  export let img: HTMLImageElement;

  function onChange(event: Event) {
    const target = event.target as HTMLInputElement;
    if (target.files?.length != 1) {
      return;
    }
    const file = target.files[0];
    const reader = new FileReader();
    reader.onload = (event) => {
      img.src = event.target?.result as string;
    };
    reader.readAsDataURL(file);
  }
</script>

<input
  on:change={onChange}
  type="file"
  name="image"
  accept=".jpg,.jpeg,.png,.bmp,.gif"
/>
