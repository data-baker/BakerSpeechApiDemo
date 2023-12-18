<template>
  <div>
    <h2 style="text-align: center;">标贝科技-TTS-DEMO</h2>

    <el-tabs type="border-card" v-model="typeTabs">
      <el-tab-pane label="v1-webSocket" name="v1ws">
        <wsv1></wsv1>
      </el-tab-pane>
      <el-tab-pane label="v2-webSocket" name="v2ws">
        <wsv2></wsv2>
      </el-tab-pane>
      <el-tab-pane label="v2-RESTfUl" name="v2RESTfUl">
        <RESTfUlV2></RESTfUlV2>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import wsv1 from './components/v1/ws/index'
import wsv2 from './components/v2/ws/index'
import RESTfUlV2 from './components/v2/RESTful/index'
export default {
  name: 'App',
  components: {
    wsv1,
    wsv2,
    RESTfUlV2
  },
  data() {
    return {
      typeTabs: 'v2ws',
    }
  },
  mounted() {},
  methods: { },
}
</script>

<style>


</style>
